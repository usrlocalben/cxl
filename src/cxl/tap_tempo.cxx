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
	if (d_first) {
		d_first = false;
		d_timer.Reset(); }
	else {
		int beatsSinceStart = d_tapCnt + 1;
		d_taps[d_headIdx] = d_timer.GetElapsed() / beatsSinceStart;
		d_tapCnt = std::clamp(d_tapCnt+1, 0, kMaxTapTempoSamples);
		d_headIdx = (d_headIdx+1) % kMaxTapTempoSamples; }}


double TapTempo::GetTempo() {
	if (d_taps.empty()) {
		throw std::runtime_error("GetTempo() called with zero taps"); }
	const auto sum = std::accumulate(begin(d_taps), begin(d_taps)+d_tapCnt, 0.0);
	const auto avgSecsPerBeat = sum / d_tapCnt;
	const auto bpm = kOneMinuteInSecs / avgSecsPerBeat;
	// Log::GetInstance().info(fmt::sprintf("TapTempo::GetTempo() sum: %.4f, avg: %.4f, bpm: %.4f", sum, avgSecsPerBeat, bpm));
	return bpm; }


void TapTempo::Reset() {
	d_first = true;
	d_tapCnt = 0;
	d_headIdx = 0; }


}  // namespace cxl
}  // namespace rqdq
