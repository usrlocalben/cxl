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
	if (d_tapCnt == 0) {
		d_timer.Reset(); }
	else {
		int idx = (d_tapCnt-1) % kMaxTapTempoSamples;
		d_samples[idx] = d_timer.GetElapsed() / d_tapCnt; }
	d_tapCnt++; }


double TapTempo::GetTempo() {
	if (d_tapCnt < 2) {
		throw std::runtime_error("GetTempo() requires at least two taps"); }
	auto n = GetNumSamples();
	auto avgBeatInSecs = std::accumulate(begin(d_samples), begin(d_samples)+n, 0.0) / n;
	auto oneMinuteInBeats = kOneMinuteInSecs / avgBeatInSecs;
	return oneMinuteInBeats; }


int TapTempo::GetNumSamples() const {
	return std::min(d_tapCnt - 1, kMaxTapTempoSamples); }


void TapTempo::Reset() {
	d_tapCnt = 0; }


}  // namespace cxl
}  // namespace rqdq
