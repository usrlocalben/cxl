#pragma once
#include "src/cxl/cxl_widget.hxx"
#include "src/rcl/rclw/rclw_winevent.hxx"
#include "src/rcl/rclw/rclw_winfile.hxx"
#include "src/rcl/rclmt/rclmt_deferred.hxx"

#include <functional>
#include <iostream>
#include <vector>

#include <Windows.h>

namespace rqdq {
namespace cxl {

using LoadFileDeferred = rclmt::Deferred<std::vector<uint8_t>&, uint32_t>;


struct ReactorEvent {
	HANDLE handle;
	std::function<void()> func;
	bool persist = false; };


class Reactor {
private:
	Reactor() = default;
public:
	static Reactor& GetInstance();

	void Run();

	void Stop() {
		d_shouldQuit = true; }

	void ListenForever(const rclw::WinEvent& we, std::function<void()> cb) {
		d_events.emplace_back(ReactorEvent{ we.Get(), cb });
		d_events.back().persist = true; }

	void ListenOnce(const rclw::WinEvent& we, std::function<void()> cb) {
		d_events.emplace_back(ReactorEvent{ we.Get(), cb });
		d_events.back().persist = false; }

	bool GetKeyState(int scanCode);

	void DrawScreen();
	void DrawScreenEventually();

	LoadFileDeferred& LoadFile(const std::string& path);
	LoadFileDeferred& LoadFile(const std::wstring& path);

	int Delay(double millis, std::function<void()> func);
	void CancelDelay(int id);

private:
	bool HandleKeyboardInput();
	bool RemoveEventByHandle(HANDLE handle) {
		auto found = std::find_if(d_events.begin(), d_events.end(),
		                          [=](auto& item) { return item.handle == handle; });
		if (found == d_events.end()) {
			return false; }  // not found is a noop
		d_events.erase(found);
		return true; }

public:
	Widget* d_widget = nullptr;

private:
	bool d_shouldQuit = false;
	rclw::WinEvent d_redrawEvent = rclw::WinEvent::MakeEvent();
	std::vector<ReactorEvent> d_events; };


}  // namespace cxl
}  // namespace rqdq
