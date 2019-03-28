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
	int GetNumSamples() const;
	int GetNumTaps() const { return tapCnt_; }

private:
	rcls::Timer timer_;
	int tapCnt_{0};
	std::array<double, kMaxTapTempoSamples> samples_; };


}  // namespace cxl
}  // namespace rqdq
