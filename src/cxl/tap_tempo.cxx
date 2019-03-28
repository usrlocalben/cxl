#include "tap_tempo.hxx"

#include <algorithm>
#include <numeric>
#include <stdexcept>

#include "src/rcl/rcls/rcls_timer.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace {

constexpr double kOneMinuteInSecs{60.0};


}  // namespace

namespace cxl {

TapTempo::TapTempo() = default;


void TapTempo::Tap() {
	if (tapCnt_ == 0) {
		timer_.Reset(); }
	else {
		int idx = (tapCnt_-1) % kMaxTapTempoSamples;
		samples_[idx] = timer_.GetElapsed() / tapCnt_; }
	tapCnt_++; }


double TapTempo::GetTempo() {
	if (tapCnt_ < 2) {
		throw std::runtime_error("GetTempo() requires at least two taps"); }
	auto n = GetNumSamples();
	auto avgBeatInSecs = std::accumulate(begin(samples_), begin(samples_)+n, 0.0) / n;
	auto oneMinuteInBeats = kOneMinuteInSecs / avgBeatInSecs;
	return oneMinuteInBeats; }


int TapTempo::GetNumSamples() const {
	return std::min(tapCnt_ - 1, kMaxTapTempoSamples); }


void TapTempo::Reset() {
	tapCnt_ = 0; }


}  // namespace cxl
}  // namespace rqdq
