#include "src/cxl/ui/splash/controller.hxx"

#include "src/rcl/rclmt/rclmt_reactor_timer.hxx"

namespace rqdq {
namespace {

constexpr float kAnimRateInHz{30.0f};

constexpr float kSplashDuration{5.0f};


}  // namespace

namespace cxl {

SplashController::SplashController(TextKit::MainLoop& loop)
	:d_loop(loop), d_view(d_t) {
	rclmt::Delay(1000.0/kAnimRateInHz, [&]() { Tick(); }); }


void SplashController::Tick() {
	d_t += 1/kAnimRateInHz;
	if (d_t > kSplashDuration) {
		onComplete.emit();
		return; }
	d_view.Invalidate();
	d_loop.DrawScreenEventually();
	rclmt::Delay(1000.0/kAnimRateInHz, [&]() { Tick(); }); }


}  // namespace cxl
}  // namespace rqdq
