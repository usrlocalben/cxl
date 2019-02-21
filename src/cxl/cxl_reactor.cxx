#include "src/cxl/cxl_reactor.hxx"

#include <functional>
#include <vector>

#define NOMINMAX
#include <Windows.h>

namespace rqdq {
namespace {

std::vector<bool> keyState(256, false);

}  // namespace
namespace cxl {

Reactor& Reactor::GetInstance() {
	static Reactor reactor;
	return reactor; }


bool Reactor::GetKeyState(int scanCode) {
	if (scanCode < 256) {
		return keyState[scanCode]; }
	return false; }


void Reactor::Run() {
	std::vector<HANDLE> pendingEvents;
	while (!d_shouldQuit) {
		pendingEvents.clear();
		pendingEvents.emplace_back(GetStdHandle(STD_INPUT_HANDLE));
		for (auto& re : d_events) {
			pendingEvents.emplace_back(re.event); }
		DWORD result = WaitForMultipleObjects(pendingEvents.size(), pendingEvents.data(), FALSE, 1000);
		if (result == WAIT_TIMEOUT) {
			// nothing
			}
		else if (WAIT_OBJECT_0 <= result && result < (WAIT_OBJECT_0+pendingEvents.size())) {
			int signaledIdx = result - WAIT_OBJECT_0;
			if (signaledIdx == 0) {
				// keyboard input
				bool handled = HandleKeyboardInput();
				if (handled && d_widget) {
					d_widget->Draw(); }}
			else {
				auto& re = d_events[signaledIdx-1];
				if (re.func) {
					re.func(); }}}}}


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
		if (d_widget) {
			handled = d_widget->HandleKeyEvent(e); }
		return handled; }
	return false; }

}  // namespace cxl
}  // namespace rqdq
