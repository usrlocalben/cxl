#include "src/cxl/tap_tempo.hxx"

#include <numeric>
#include <stdexcept>

#include "src/cxl/log.hxx"
#include "src/rcl/rcls/rcls_timer.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace {

constexpr int kMaxSamples = 16;


}  // namespace

namespace cxl {

TapTempo::TapTempo() = default;


void TapTempo::Tap() {
	if (d_first) {
		d_first = false;
		d_timer.Reset(); }
	else {
		d_buf.push_back(d_timer.GetElapsed());
		while (d_buf.size() > kMaxSamples) {
			d_buf.pop_front(); }
		d_timer.Reset(); }}


double TapTempo::GetTempo() {
	if (d_buf.empty()) {
		throw std::runtime_error("GetTempo() called with zero taps"); }
	const auto sum = std::accumulate(begin(d_buf), end(d_buf), 0.0);
	const auto avgSecondsPerBeat = sum / d_buf.size();
	const auto bpm = 60.0 / avgSecondsPerBeat;
	// Log::GetInstance().info(fmt::sprintf("TapTempo::GetTempo() sum: %.4f, avg: %.4f, bpm: %.4f", sum, avgSecondsPerBeat, bpm));
	return bpm; }


void TapTempo::Reset() {
	d_first = true;
	d_buf.clear(); }


}  // namespace cxl
}  // namespace rqdq
