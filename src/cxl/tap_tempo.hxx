#pragma once

#include <array>

#include "src/rcl/rcls/rcls_timer.hxx"

namespace rqdq {
namespace cxl {

constexpr int kMaxTapTempoSamples{16};

class TapTempo {
public:
	TapTempo();
	TapTempo(const TapTempo&) = delete;
	TapTempo& operator=(const TapTempo&) = delete;

	void Tap();
	void Reset();
	double GetTempo();
	int GetNumSamples() const { return d_taps.size(); }

private:
	rcls::Timer d_timer;
	bool d_first{true};
	int d_tapCnt{0};
	int d_headIdx{0};
	std::array<double, kMaxTapTempoSamples> d_taps{}; };


}  // namespace cxl
}  // namespace rqdq
