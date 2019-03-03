#include "src/cxl/cxl_reactor.hxx"

#include "src/cxl/cxl_log.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

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
	cxl::WindowsEvent event; };

std::vector<DelayInfo> delays;

int delaySeq = 1;

std::pair<int, cxl::WindowsEvent&> GetDelay() {
	const int id = delaySeq++;
	for (int i=0; i<delays.size(); i++) {
		auto& item = delays[i];
		if (!item.inUse) {
			item.inUse = true;
			item.id = delaySeq;
			return {i, item.event}; }}
	delays.emplace_back(DelayInfo{true, false, id, cxl::WindowsEvent::MakeTimer()});
	return {id, delays.back().event}; };


bool ReleaseTimer(HANDLE h) {
	auto found = std::find_if(delays.begin(), delays.end(), [=](auto& item) { return item.event.GetHandle() == h; });
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
		:request(other.request),
		fd(other.fd),
		id(other.id),
		buffer(std::move(other.buffer)),
		good(other.good),
		bytesRead(other.bytesRead),
		expectedSizeInBytes(other.expectedSizeInBytes),
		onComplete(std::move(other.onComplete)),
		onError(std::move(other.onError))
	{
		other.request.hEvent = nullptr;
		other.fd = nullptr; }

	OVERLAPPED request;
	HANDLE fd = nullptr;
	int id = -1;
	std::vector<uint8_t> buffer;
	bool good = true;
	DWORD error;
	DWORD bytesRead;
	int64_t expectedSizeInBytes;
	std::function<void(const std::vector<uint8_t>&)> onComplete;
	std::function<void(uint32_t)> onError;

	~FileOp() {
		if (fd != nullptr) {
			CloseHandle(fd);
			fd = nullptr; }
		if (request.hEvent != nullptr) {
			CloseHandle(request.hEvent);
			request.hEvent = nullptr; }} };

std::list<FileOp> fileOps;
int fileOpSeq = 0;

int GetNextOpId() { return fileOpSeq++; };

void onFileOpError(int id) {
	auto op = std::find_if(begin(fileOps), end(fileOps),
	                       [=](auto& item) { return item.id == id; });
	if (op == fileOps.end()) {
		cxl::Log::GetInstance().warn(fmt::sprintf("fileOp id=%d not found when error", id)); }

	if (op->onError) {
		op->onError(op->error); }
	fileOps.erase(op); }


void onFileOpComplete(int id) {
	auto op = std::find_if(begin(fileOps), end(fileOps),
	                       [=](auto& item) { return item.id == id; });
	if (op == fileOps.end()) {
		cxl::Log::GetInstance().warn(fmt::sprintf("fileOp id=%d not found when complete", id)); }

	if (op->onComplete) {
		op->onComplete(op->buffer); }
	fileOps.erase(op); }


void onFileOpEvent(int id) {
	auto op = std::find_if(begin(fileOps), end(fileOps),
	                       [=](auto& item) { return item.id == id; });
	if (op == fileOps.end()) {
		cxl::Log::GetInstance().warn(fmt::sprintf("fileOp id=%d not found when event", id)); }

	auto success = GetOverlappedResult(op->fd, &op->request, &op->bytesRead, FALSE);
	if (success == 0) {
		auto error = GetLastError();
		auto msg = fmt::sprintf("GetOverlappedResult unexpected error %d", error);
		throw std::runtime_error(msg); }

	if (op->bytesRead != op->expectedSizeInBytes) {
		auto msg = fmt::sprintf("expected %d bytes but read %d", op->expectedSizeInBytes, op->bytesRead);
		throw std::runtime_error(msg); }

	if (op->onComplete) {
		op->onComplete(op->buffer); }
	fileOps.erase(op); }


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
	d_redrawEvent.Set(); }


void Reactor::Run() {
	ListenForever(ReactorEvent{ d_redrawEvent.GetHandle(),
				  [&](){ DrawScreen(); }});

	std::vector<HANDLE> pendingEvents;
	while (!d_shouldQuit) {
		pendingEvents.clear();
		pendingEvents.emplace_back(GetStdHandle(STD_INPUT_HANDLE));
		for (auto& re : d_events) {
			pendingEvents.emplace_back(re.handle); }
		DWORD result = WaitForMultipleObjects(pendingEvents.size(), pendingEvents.data(), FALSE, 1000);
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


void Reactor::LoadFile(const std::string& path, std::function<void(const std::vector<uint8_t>&)> onComplete, std::function<void(uint32_t)> onError) {
	auto tmp = rclt::UTF8Codec::Decode(path);
	LoadFile(tmp, onComplete, onError);}


void Reactor::LoadFile(const std::wstring& path, std::function<void(const std::vector<uint8_t>&)> onComplete, std::function<void(uint32_t)> onError) {
	fileOps.emplace_back(FileOp{});
	auto& op = fileOps.back();

	op.id = GetNextOpId();
	op.onComplete = onComplete;
	op.onError = onError;
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
	if (op.fd == INVALID_HANDLE_VALUE) {
		op.error = GetLastError();
		op.fd = nullptr;
		// Log::GetInstance().info(fmt::sprintf("CreateFileW() failed with %d", op.error));
		op.good = false;
		op.request.hEvent = CreateEventW(nullptr, TRUE, TRUE, nullptr);
		ListenOnce(ReactorEvent{ op.request.hEvent, [&](){ onFileOpError(op.id); } });
		return; }

	auto result = GetFileSizeEx(op.fd, reinterpret_cast<LARGE_INTEGER*>(&op.expectedSizeInBytes));
	if (result == 0) {
		op.error = GetLastError();
		// Log::GetInstance().info(fmt::sprintf("GetFileSizeEx() failed with %d", op.error));
		op.good = false;
		op.request.hEvent = CreateEventW(nullptr, TRUE, TRUE, nullptr);
		ListenOnce(ReactorEvent{ op.request.hEvent, [&](){ onFileOpError(op.id); } });
		return; }

	op.buffer.resize(op.expectedSizeInBytes);
	op.request.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
	if (op.request.hEvent == nullptr) {
		auto err = GetLastError();
		auto msg = fmt::sprintf("CreateEventW failed with %d", err);
		throw std::runtime_error(msg); }

	result = ReadFile(op.fd,
	                  op.buffer.data(),
	                  op.expectedSizeInBytes,
	                  &op.bytesRead,
	                  &op.request);
	op.error = GetLastError();

	if (result == 0 && op.error != ERROR_IO_PENDING) {
		// error...
		// Log::GetInstance().info(fmt::sprintf("ReadFile failed with %d", op.error));
		CloseHandle(op.request.hEvent);
		op.request.hEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);
		ListenOnce(ReactorEvent{ op.request.hEvent,
		                         [&](){ onFileOpError(op.id); }});
		return; }

	if (result != 0) {
		// Log::GetInstance().warn(fmt::sprintf("ReadFile was synchronous! read %d bytes", op.bytesRead));
		// read was processed synchronously
		op.error = 0;
		CloseHandle(op.request.hEvent);
		op.request.hEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);
		ListenOnce(ReactorEvent{ op.request.hEvent, [&](){ onFileOpComplete(op.id); } });
		return; }

	// request submitted successfully, wait for results
	op.error = 0;
	// Log::GetInstance().info(fmt::sprintf("ReadFile success, listening"));
	ListenOnce(ReactorEvent{ op.request.hEvent, [&](){ onFileOpEvent(op.id); } }); }


/**
 * Delay()
 * similar to Twisted callLater() or JavaScript's setTimeout()
 */
int Reactor::Delay(const double millis, const std::function<void()> func) {
	auto [id, timer] = GetDelay();
	timer.SetIn(millis);
	auto rawHandle = timer.GetHandle();
	ListenOnce(ReactorEvent{ rawHandle, [=](){
		bool canceled = ReleaseTimer(rawHandle);
		if (!canceled) {
			func(); }
		} });
	return id; };


void Reactor::CancelDelay(int id) {
	auto found = std::find_if(delays.begin(), delays.end(), [=](auto& item) { return item.id = id; });
	if (found == delays.end()) {
		return; }  // not found is a no-op
	
	found->canceled = true;
	auto handle = found->event.GetHandle();
	auto result = CancelWaitableTimer(handle);
	if (result != 0) {
		auto error = GetLastError();
		auto msg = fmt::sprintf("CancelWaitableTimer error %d", error);
		throw std::runtime_error(msg); }
	RemoveEventByHandle(handle);
	found->inUse = false; }


}  // namespace cxl
}  // namespace rqdq
