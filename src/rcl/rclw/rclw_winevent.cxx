#include "src/rcl/rclw/rclw_winevent.hxx"

#include <stdexcept>

#include "3rdparty/fmt/include/fmt/printf.h"
#include <Windows.h>

namespace rqdq {
namespace rclw {


void WinEvent::SignalIn(double millis) {
	const auto t = static_cast<int64_t>(-millis * 10000);
	const auto result = SetWaitableTimer(d_handle, reinterpret_cast<const LARGE_INTEGER*>(&t), 0, NULL, NULL, 0);
	if (result == 0) {
		const auto error = GetLastError();
		const auto msg = fmt::sprintf("SetWaitableTimer error %d", error);
		throw std::runtime_error(msg); }}


WinEvent WinEvent::MakeEvent(bool initialState /*=false*/) {
	const auto event = CreateEventW(nullptr, FALSE, initialState, nullptr);
	if (event == nullptr) {
		const auto error = GetLastError();
		const auto msg = fmt::sprintf("CreateEventW error %d", error);
		throw std::runtime_error(msg); }
	auto instance = WinEvent(event);
	return instance; }


WinEvent WinEvent::MakeTimer() {
	const auto event = CreateWaitableTimerW(nullptr, FALSE, nullptr);
	if (event == nullptr) {
		const auto error = GetLastError();
		const auto msg = fmt::sprintf("CreateWaitableTimerW error %d", error);
		throw std::runtime_error(msg); }
	auto instance = WinEvent(event);
	return instance; }

}  // namespace rclw
}  // namespace rqdq
