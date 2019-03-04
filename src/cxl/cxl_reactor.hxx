#pragma once
#include "src/cxl/cxl_widget.hxx"

#include <functional>
#include <iostream>
#include <vector>

#include "3rdparty/fmt/include/fmt/printf.h"
#include <Windows.h>

namespace rqdq {
namespace cxl {

template <typename GoodT, typename BadT>
struct Deferred {
	using goodfunc = std::function<void(GoodT)>;
	using badfunc = std::function<void(BadT)>;

	void Callback(GoodT data) {
		if (callback) {
			callback(data); }}

	void Errback(BadT data) {
		if (errback) {
			errback(data); }}

	void AddCallbacks(goodfunc f1, badfunc f2) {
		callback = std::move(f1);
		errback = std::move(f2); }

	goodfunc callback;
	badfunc errback; };


using LoadFileDeferred = Deferred<std::vector<uint8_t>&, uint32_t>;


class WinFile {
public:
	// lifecycle
	WinFile(HANDLE fd=INVALID_HANDLE_VALUE) :d_fd(fd) {}
	WinFile(const WinFile&) = delete;
	WinFile& operator=(const WinFile&) = delete;
	WinFile(WinFile&& other) noexcept {
		other.Swap(*this); }
	WinFile& operator=(WinFile&& other) noexcept {
		other.Swap(*this);
		return *this; }
	~WinFile() {
		Close(); }
	HANDLE Get() const {
		return d_fd;}
	void Swap(WinFile& other) noexcept {
		std::swap(d_fd, other.d_fd); }
	void Reset(HANDLE fd = INVALID_HANDLE_VALUE) {
		WinFile tmp(fd);
		tmp.Swap(*this); }
	void Close() {
		if (d_fd != INVALID_HANDLE_VALUE) {
			const auto fd = d_fd;
			d_fd = INVALID_HANDLE_VALUE;
			CloseHandle(d_fd); }}

	// actions

private:
	HANDLE d_fd = INVALID_HANDLE_VALUE; };


class WinEvent {
public:
	// lifecycle
	WinEvent(HANDLE event=nullptr) :d_handle(event) {}
	WinEvent(const WinEvent& other) = delete;
	WinEvent& operator=(const WinEvent& other) = delete;
	WinEvent(WinEvent&& other) noexcept {
		other.Swap(*this); }
	WinEvent& operator=(WinEvent&& other) noexcept {
		other.Swap(*this);
		return *this; }
	void Swap(WinEvent& other) noexcept {
		std::swap(d_handle, other.d_handle); }
	~WinEvent() {
		if (d_handle != nullptr) {
			CloseHandle(d_handle); }}
	HANDLE Release() {
		auto tmp = d_handle;
		d_handle = nullptr;
		return tmp; }
	HANDLE Get() const {
		assert(d_handle != nullptr);
		return d_handle; }

	// actions
	void Signal() {
		SetEvent(d_handle); }

	void SignalIn(double millis) {
		const auto t = static_cast<int64_t>(-millis * 10000);
		const auto result = SetWaitableTimer(d_handle, reinterpret_cast<const LARGE_INTEGER*>(&t), 0, NULL, NULL, 0);
		if (result == 0) {
			const auto error = GetLastError();
			const auto msg = fmt::sprintf("SetWaitableTimer error %d", error);
			throw std::runtime_error(msg); }}

	// factories
	static WinEvent MakeEvent(bool initialState=false) {
		const auto event = CreateEventW(nullptr, FALSE, initialState, nullptr);
		if (event == nullptr) {
			const auto error = GetLastError();
			const auto msg = fmt::sprintf("CreateEventW error %d", error);
			throw std::runtime_error(msg); }
		auto instance = WinEvent(event);
		return instance; }

	static WinEvent MakeTimer() {
		const auto event = CreateWaitableTimerW(nullptr, FALSE, nullptr);
		if (event == nullptr) {
			const auto error = GetLastError();
			const auto msg = fmt::sprintf("CreateWaitableTimerW error %d", error);
			throw std::runtime_error(msg); }
		auto instance = WinEvent(event);
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

	void ListenForever(const WinEvent& we, std::function<void()> cb) {
		d_events.emplace_back(ReactorEvent{ we.Get(), cb });
		d_events.back().persist = true; }

	void ListenOnce(const WinEvent& we, std::function<void()> cb) {
		d_events.emplace_back(ReactorEvent{ we.Get(), cb });
		d_events.back().persist = false; }

	bool GetKeyState(int scanCode);

	void DrawScreen();
	void DrawScreenEventually();

	std::shared_ptr<LoadFileDeferred> LoadFile(const std::string& path);
	std::shared_ptr<LoadFileDeferred> LoadFile(const std::wstring& path);

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
	WinEvent d_redrawEvent = WinEvent::MakeEvent();
	std::vector<ReactorEvent> d_events; };


}  // namespace cxl
}  // namespace rqdq
