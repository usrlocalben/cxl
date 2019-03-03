#pragma once
#include "src/cxl/cxl_widget.hxx"

#include <functional>
#include <iostream>
#include <vector>
#include <Windows.h>
#include "3rdparty/fmt/include/fmt/printf.h"

namespace rqdq {
namespace cxl {


class WindowsEvent {
public:
	// default constructable
	WindowsEvent() = default;

	// make from owned handle
	WindowsEvent(HANDLE event) :d_handle(event) {}

	// not copyable
	WindowsEvent(const WindowsEvent& other) = delete;
	WindowsEvent& operator=(const WindowsEvent& other) = delete;

	// movable
	WindowsEvent(WindowsEvent&& other) noexcept {
		d_handle = other.d_handle;
		other.d_handle = nullptr; }
	WindowsEvent& operator=(WindowsEvent&& other) noexcept {
		d_handle = other.d_handle;
		other.d_handle = nullptr;
		return *this; }

	~WindowsEvent() {
		if (d_handle != nullptr) {
			CloseHandle(d_handle); }}

	void Set() {
		SetEvent(d_handle); }

	void SetIn(double millis) {
		const auto t = static_cast<int64_t>(-millis * 10000);
		const auto result = SetWaitableTimer(d_handle, reinterpret_cast<const LARGE_INTEGER*>(&t), 0, NULL, NULL, 0);
		if (result == 0) {
			const auto error = GetLastError();
			const auto msg = fmt::sprintf("SetWaitableTimer error %d", error);
			throw std::runtime_error(msg); }}

	HANDLE Release() {
		auto tmp = d_handle;
		d_handle = nullptr;
		return tmp; }

	HANDLE GetHandle() {
		assert(d_handle != nullptr);
		return d_handle; }

	static WindowsEvent MakeEvent() {
		const auto event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
		if (event == nullptr) {
			const auto error = GetLastError();
			const auto msg = fmt::sprintf("CreateEventW error %d", error);
			throw std::runtime_error(msg); }
		auto instance = WindowsEvent(event);
		return instance; }

	static WindowsEvent MakeTimer() {
		const auto event = CreateWaitableTimerW(nullptr, FALSE, nullptr);
		if (event == nullptr) {
			const auto error = GetLastError();
			const auto msg = fmt::sprintf("CreateWaitableTimerW error %d", error);
			throw std::runtime_error(msg); }
		auto instance = WindowsEvent(event);
		return instance; }

private:
	HANDLE d_handle = nullptr; };


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

	void ListenForever(ReactorEvent re) {
		d_events.emplace_back(re);
		d_events.back().persist = true; }

	void ListenOnce(ReactorEvent re) {
		d_events.emplace_back(re);
		d_events.back().persist = false; }

	bool GetKeyState(int scanCode);

	void DrawScreen();
	void DrawScreenEventually();

	void LoadFile(const std::string& path, std::function<void(const std::vector<uint8_t>&)> onComplete, std::function<void(uint32_t)> onError);
	void LoadFile(const std::wstring& path, std::function<void(const std::vector<uint8_t>&)> onComplete, std::function<void(uint32_t)> onError);

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
	WindowsEvent d_redrawEvent = WindowsEvent::MakeEvent();
	std::vector<ReactorEvent> d_events; };


}  // namespace cxl
}  // namespace rqdq
