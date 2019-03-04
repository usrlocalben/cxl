#pragma once
#include <algorithm>

#include <Windows.h>

namespace rqdq {
namespace rclw {

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

private:
	HANDLE d_fd = INVALID_HANDLE_VALUE; };

}  // namespace rclw
}  // namespace rqdq
