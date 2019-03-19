#include "src/cxl/tap_tempo.hxx"

#include <algorithm>
#include <numeric>
#include <stdexcept>

#include "src/cxl/log.hxx"
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
	const auto numSamples = std::min(d_tapCnt-1, kMaxTapTempoSamples);
	const auto sum = std::accumulate(begin(d_samples), begin(d_samples)+numSamples, 0.0);
	const auto avgSecsPerBeat = sum / numSamples;
	const auto bpm = kOneMinuteInSecs / avgSecsPerBeat;
	// Log::GetInstance().info(fmt::sprintf("TapTempo::GetTempo() sum: %.4f, avg: %.4f, bpm: %.4f", sum, avgSecsPerBeat, bpm));
	return bpm; }


void TapTempo::Reset() {
	d_tapCnt = 0; }


}  // namespace cxl
}  // namespace rqdq
