#pragma once
#include "src/cxl/cxl_widget.hxx"

#include <functional>
#include <vector>
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
	std::function<void()> func;
	bool keep = false; };


class Reactor {
private:
	Reactor() = default;
public:
	static Reactor& GetInstance();

	void Run();

	void Stop() {
		d_shouldQuit = true; }

	void ListenForever(ReactorEvent re) {
		d_events.emplace_back(re);
		d_events.back().keep = true; }

	void ListenOnce(ReactorEvent re) {
		d_events.emplace_back(re);
		d_events.back().keep = false; }

	bool GetKeyState(int scanCode);

	void DrawScreen();
	void DrawScreenEventually();

	void LoadFile(const std::string& path, std::function<void(const std::vector<uint8_t>&)> onComplete, std::function<void(uint32_t)> onError);
	void LoadFile(const std::wstring& path, std::function<void(const std::vector<uint8_t>&)> onComplete, std::function<void(uint32_t)> onError);

	void Delay(double millis, std::function<void()> func);

private:
	bool HandleKeyboardInput();

public:
	Widget* d_widget = nullptr;

private:
	bool d_shouldQuit = false;
	WindowsEvent d_redrawEvent;
	std::vector<ReactorEvent> d_events; };


}  // namespace cxl
}  // namespace rqdq
