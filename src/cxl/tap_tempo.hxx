#pragma once

#include <deque>

#include "src/rcl/rcls/rcls_timer.hxx"

namespace rqdq {
namespace cxl {

class TapTempo {
public:
	TapTempo();
	void Tap();
	void Reset();
	double GetTempo();
	int GetNumSamples() const { return d_buf.size(); }
private:
	rcls::Timer d_timer;
	bool d_first{true};
	std::deque<double> d_buf; };


}  // namespace cxl
}  // namespace rqdq
