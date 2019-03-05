#pragma once

#include <algorithm>

#include "3rdparty/fmt/include/fmt/printf.h"
#include <Windows.h>

namespace rqdq {
namespace rclw {


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
		Close(); }
	void Close() {
		if (d_handle != nullptr) {
			const auto tmp = d_handle;
			d_handle = nullptr;
			CloseHandle(tmp); }}
	HANDLE Release() {
		const auto tmp = d_handle;
		d_handle = nullptr;
		return tmp; }
	HANDLE Get() const {
		assert(d_handle != nullptr);
		return d_handle; }

	// actions
	void Signal() { SetEvent(d_handle); }
	void SignalIn(double millis);

	// factories
	static WinEvent MakeEvent(bool initialState=false);
	static WinEvent MakeTimer();

private:
	HANDLE d_handle = nullptr; };


}  // namespace rclw
}  // namespace rqdq
