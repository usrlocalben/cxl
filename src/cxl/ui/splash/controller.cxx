#include "controller.hxx"

#include "src/cxl/ui/splash/view.hxx"
#include "src/rcl/rclmt/rclmt_reactor_timer.hxx"

namespace rqdq {
namespace {

constexpr float kAnimRateInHz{30.0F};

constexpr float kSplashDuration{5.0F};


}  // namespace

namespace cxl {

SplashController::SplashController(TextKit::MainLoop& loop)
	:d_loop(loop), d_view{MakeSplashView(d_t)} {
	rclmt::Delay(1000.0/kAnimRateInHz, [&]() { Tick(); }); }


void SplashController::Tick() {
	d_t += 1/kAnimRateInHz;
	if (d_t > kSplashDuration) {
		onComplete.emit();
		return; }
	d_loop.DrawScreenEventually();
	rclmt::Delay(1000.0/kAnimRateInHz, [&]() { Tick(); }); }


}  // namespace cxl
}  // namespace rqdq
