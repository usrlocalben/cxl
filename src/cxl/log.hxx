#pragma once
#include <functional>
#include <string>
#include <vector>

#include "src/rcl/rclmt/rclmt_signal.hxx"

namespace rqdq {
namespace cxl {

class Log {
private:
	Log();

public:
	static Log& GetInstance();
	void warn(const std::string& msg);
	void info(const std::string& msg);

	int GetHeadIdx() {
		return head_; }

	const std::string& GetEntry(int relIdx, int from = 0x3f3f3f3f) {
		if (from == 0x3f3f3f3f) {
			from = GetHeadIdx(); }
		int ri = from - 1 - relIdx;
		if (ri < 0) {
			ri += 1024; }
		return buf_[ri]; }

public:
	rclmt::Signal<void()> updated_;

private:
	int head_ = 0;
	std::vector<std::string> buf_; };


}  // namespace cxl
}  // namespace rqdq
