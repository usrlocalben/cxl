#pragma once
#include <functional>
#include <string>
#include <vector>
#include "3rdparty/wink/wink/signal.hpp"
#include "3rdparty/wink/wink/slot.hpp"

namespace rqdq {
namespace cxl {

class Log {
private:
	Log() {
		d_buf.resize(1024); }
public:
	static Log& GetInstance() {
		static Log log;
		return log; }

	void warn(const std::string& msg) {
		return info(msg); }

	void info(const std::string& msg) {
		d_buf[d_head].assign(msg);
		d_head++;
		if (d_head >= 1024) {
			d_head = 0; }
		d_updated.emit(); }

	int GetHeadIdx() {
		return d_head; }

	const std::string& GetEntry(int relIdx, int from = 0x3f3f3f3f) {
		if (from == 0x3f3f3f3f) {
			from = GetHeadIdx(); }
		int ri = from - 1 - relIdx;
		if (ri < 0) {
			ri += 1024; }
		return d_buf[ri]; }

public:
	//wink::signal<wink::slot<void()>> d_updated;
	wink::signal<std::function<void()>> d_updated;
private:
	int d_head = 0;
	std::vector<std::string> d_buf; };

}  // namespace cxl
}  // namespace rqdq
