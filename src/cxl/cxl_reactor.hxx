#pragma once
#include "src/cxl/cxl_widget.hxx"

#include <functional>
#include <vector>

#define NOMINMAX
#include <Windows.h>

namespace rqdq {
namespace cxl {


class WindowsEvent {
public:
	WindowsEvent() {
		d_event = CreateEventW(nullptr, FALSE, FALSE, nullptr); }
	WindowsEvent(const WindowsEvent& other) = delete;
	WindowsEvent(WindowsEvent&& other) = delete;
	WindowsEvent& operator=(const WindowsEvent& other) = delete;
	~WindowsEvent() { CloseHandle(d_event); }
	void Set() { SetEvent(d_event); }
	HANDLE GetHandle() { return d_event; }
private:
	HANDLE d_event; };
struct ReactorEvent {
	HANDLE event;
	std::function<void()> func; };


class Reactor {
private:
	Reactor() = default;
public:
	static Reactor& GetInstance();

	void Run();

	void Stop() {
		d_shouldQuit = true; }

	void AddEvent(ReactorEvent re) {
		d_events.emplace_back(re); }

	bool GetKeyState(int scanCode);
private:
	bool HandleKeyboardInput();

public:
	Widget* d_widget = nullptr;

private:
	bool d_shouldQuit = false;
	std::vector<ReactorEvent> d_events; };


}  // namespace cxl
}  // namespace rqdq
