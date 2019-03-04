#include "src/cxl/cxl_reactor.hxx"

#include "src/cxl/cxl_log.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rclw/rclw_winfile.hxx"

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include "3rdparty/fmt/include/fmt/printf.h"
#include <Windows.h>

namespace rqdq {
namespace {

// delay
struct DelayInfo {
	bool inUse = false;
	bool canceled = false;
	int id = 0;
	cxl::WinEvent event; };

std::vector<DelayInfo> delays;

int delaySeq = 1;

std::pair<int, cxl::WinEvent&> GetDelay() {
	const int id = delaySeq++;
	for (int i=0; i<delays.size(); i++) {
		auto& item = delays[i];
		if (!item.inUse) {
			item.inUse = true;
			item.id = delaySeq;
			return {i, item.event}; }}
	delays.emplace_back(DelayInfo{true, false, id, cxl::WinEvent::MakeTimer()});
	return {id, delays.back().event}; };


bool ReleaseTimer(HANDLE h) {
	auto found = std::find_if(delays.begin(), delays.end(), [=](auto& item) { return item.event.Get() == h; });
	if (found == delays.end()) {
		throw std::runtime_error("ReleaseTimer called with handle not found in delays collection"); }
	bool canceled = found->canceled;
	found->inUse = false;
	found->id = 0;
	found->canceled = false;
	return canceled; }


// keyboard i/o
std::vector<bool> keyState(256, false);


// file i/o
struct FileOp {

	FileOp() = default;
	FileOp(const FileOp&) = delete;
	FileOp& operator=(const FileOp&) = delete;

	FileOp(FileOp&& other) noexcept
		:fd(std::move(other.fd)),
		event(std::move(other.event)),
		request(other.request),
		id(other.id),
		buffer(std::move(other.buffer)),
		good(other.good),
		expectedSizeInBytes(other.expectedSizeInBytes),
		deferred(std::move(other.deferred)) {}

	int id = -1;
	rclw::WinFile fd;
	cxl::WinEvent event;
	std::shared_ptr<cxl::LoadFileDeferred> deferred;

	bool good = true;
	int64_t expectedSizeInBytes;
	DWORD error;
	std::vector<uint8_t> buffer;
	OVERLAPPED request; };


std::list<FileOp> fileOps;
int fileOpSeq = 0;
int GetNextOpId() { return fileOpSeq++; };
auto FindOpById(int id) {
	return std::find_if(begin(fileOps), end(fileOps),
	                    [=](auto& item) { return item.id == id; }); }

void onFileOpError(int id) {
	if (auto op = FindOpById(id); op!=fileOps.end()) {
		op->deferred->Errback(op->error);
		fileOps.erase(op); }
	else {
		cxl::Log::GetInstance().warn(fmt::sprintf("fileOp id=%d not found when error", id)); }}


void onFileOpComplete(int id) {
	if (auto op = FindOpById(id); op!=fileOps.end()) {
		op->deferred->Callback(op->buffer);
		fileOps.erase(op); }
	else {
		cxl::Log::GetInstance().warn(fmt::sprintf("fileOp id=%d not found when complete", id)); }}


void onFileOpEvent(int id) {
	auto search = FindOpById(id);
	if (search == fileOps.end()) {
		cxl::Log::GetInstance().warn(fmt::sprintf("fileOp id=%d not found when event", id));
		return; }

	FileOp& op = *search;
	DWORD bytesRead = 0;

	auto success = GetOverlappedResult(op.fd.Get(), &op.request, &bytesRead, FALSE);
	if (success == 0) {
		auto error = GetLastError();
		auto msg = fmt::sprintf("GetOverlappedResult unexpected error %d", error);
		throw std::runtime_error(msg); }

	if (bytesRead != op.expectedSizeInBytes) {
		auto msg = fmt::sprintf("expected %d bytes but read %d", op.expectedSizeInBytes, bytesRead);
		throw std::runtime_error(msg); }

	FileOp foo(std::move(op));
	fileOps.erase(search);
	foo.deferred->Callback(foo.buffer); }


}  // namespace
namespace cxl {

Reactor& Reactor::GetInstance() {
	static Reactor reactor;
	return reactor; }


bool Reactor::GetKeyState(int scanCode) {
	if (scanCode < 256) {
		return keyState[scanCode]; }
	return false; }


void Reactor::DrawScreen() {
	if (d_widget != nullptr) {
		auto canvas = d_widget->Draw(80, 25);
		SMALL_RECT rect;
		rect.Left = 0;
		rect.Top = 0;
		rect.Right = canvas.d_width - 1;
		rect.Bottom = canvas.d_height - 1;
		auto result = WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),
		                                 canvas.GetDataPtr(),
		                                 COORD{ short(canvas.d_width), short(canvas.d_height) },
		                                 COORD{ 0, 0 },
		                                 &rect);
		if (result == 0) {
			throw std::runtime_error("WriteConsoleOutput failure"); }}}


void Reactor::DrawScreenEventually() {
	d_redrawEvent.Signal(); }


void Reactor::Run() {
	ListenForever(d_redrawEvent, [&](){ DrawScreen(); });

	std::vector<HANDLE> pendingEvents;
	while (!d_shouldQuit) {
		pendingEvents.clear();
		pendingEvents.emplace_back(GetStdHandle(STD_INPUT_HANDLE));
		for (auto& re : d_events) {
			pendingEvents.emplace_back(re.handle); }
		DWORD result = WaitForMultipleObjects(static_cast<DWORD>(pendingEvents.size()),
		                                      pendingEvents.data(), FALSE, 1000);
		if (result == WAIT_TIMEOUT) {
			// nothing
			}
		else if (WAIT_OBJECT_0 <= result && result < (WAIT_OBJECT_0+pendingEvents.size())) {
			int signaledIdx = result - WAIT_OBJECT_0;
			if (signaledIdx == 0) {
				// keyboard input
				bool handled = HandleKeyboardInput();
				if (handled) {
					DrawScreen(); }}
			else {
				const int eventIdx = signaledIdx - 1;
				auto& re = d_events[eventIdx];
				if (re.func) {
					re.func(); }
				if (!re.persist) {
					d_events.erase(d_events.begin() + eventIdx); }}}
		else {
			auto err = GetLastError();
			throw std::runtime_error(fmt::sprintf("WaitForMultipleObjects error %d", err)); }}}


bool Reactor::HandleKeyboardInput() {
	INPUT_RECORD record;
	DWORD numRead;
	if (ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &record, 1, &numRead) == 0) {
		throw std::runtime_error("ReadConsoleInput failure"); }
	if (record.EventType == KEY_EVENT) {
		auto& e = record.Event.KeyEvent;

		if (e.wVirtualScanCode<256) {
			keyState[e.wVirtualScanCode] = (e.bKeyDown != 0); }

		bool handled = false;
		if (d_widget != nullptr) {
			handled = d_widget->HandleKeyEvent(e); }
		return handled; }
	return false; }


std::shared_ptr<LoadFileDeferred> Reactor::LoadFile(const std::string& path) {
	auto tmp = rclt::UTF8Codec::Decode(path);
	return LoadFile(tmp); }


std::shared_ptr<LoadFileDeferred> Reactor::LoadFile(const std::wstring& path) {
	fileOps.emplace_back(FileOp{});
	auto& op = fileOps.back();
	op.deferred = std::make_shared<LoadFileDeferred>();
	auto& d = op.deferred;

	op.id = GetNextOpId();
	op.good = true;
	op.error = 0;
	memset(&op.request, 0, sizeof(op.request));

	op.fd = CreateFileW(path.c_str(),
	                    GENERIC_READ,
	                    FILE_SHARE_READ,
	                    nullptr,
	                    OPEN_EXISTING,
	                    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
	                    nullptr);
	if (op.fd.Get() == INVALID_HANDLE_VALUE) {
		op.error = GetLastError();
		// Log::GetInstance().info(fmt::sprintf("CreateFileW() failed with %d", op.error));
		op.good = false;
		op.event = WinEvent::MakeEvent(true);
		ListenOnce(op.event, [&](){ onFileOpError(op.id); });
		return d; }

	auto result = GetFileSizeEx(op.fd.Get(), reinterpret_cast<LARGE_INTEGER*>(&op.expectedSizeInBytes));
	if (result == 0) {
		op.error = GetLastError();
		// Log::GetInstance().info(fmt::sprintf("GetFileSizeEx() failed with %d", op.error));
		op.good = false;
		op.event = WinEvent::MakeEvent(true);
		ListenOnce(op.event, [&](){ onFileOpError(op.id); });
		return d; }

	op.buffer.resize(op.expectedSizeInBytes);
	op.event = WinEvent::MakeEvent(false);
	op.request.hEvent = op.event.Get();

	DWORD bytesRead;

	result = ReadFile(op.fd.Get(),
	                  op.buffer.data(),
	                  op.expectedSizeInBytes,
	                  &bytesRead,
	                  &op.request);
	op.error = GetLastError();

	if (result == 0 && op.error != ERROR_IO_PENDING) {
		// error...
		op.event = WinEvent::MakeEvent(true);
		ListenOnce(op.event, [&](){ onFileOpError(op.id); });
		return d; }

	if (result != 0) {
		// read was processed synchronously
		op.error = 0;
		op.event = WinEvent::MakeEvent(true);
		ListenOnce(op.event, [&](){ onFileOpComplete(op.id); });
		return d; }

	// request submitted successfully, wait for results
	op.error = 0;
	// Log::GetInstance().info(fmt::sprintf("ReadFile success, listening"));
	ListenOnce(op.event, [&](){ onFileOpEvent(op.id); });
	return d; }


/**
 * Delay()
 * similar to Twisted callLater() or JavaScript's setTimeout()
 */
int Reactor::Delay(const double millis, const std::function<void()> func) {
	auto [id, timer] = GetDelay();
	timer.SignalIn(millis);
	auto rawHandle = timer.Get();
	ListenOnce(timer, [=](){
		bool canceled = ReleaseTimer(rawHandle);
		if (!canceled) {
			func(); }
		});
	return id; };


void Reactor::CancelDelay(int id) {
	auto found = std::find_if(delays.begin(), delays.end(), [=](auto& item) { return item.id = id; });
	if (found == delays.end()) {
		return; }  // not found is a no-op

	found->canceled = true;
	auto handle = found->event.Get();
	auto result = CancelWaitableTimer(handle);
	if (result != 0) {
		auto error = GetLastError();
		auto msg = fmt::sprintf("CancelWaitableTimer error %d", error);
		throw std::runtime_error(msg); }
	RemoveEventByHandle(handle);
	found->inUse = false; }


}  // namespace cxl
}  // namespace rqdq
