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
	:loop_(loop), view_{MakeSplashView(t_)} {
	rclmt::Delay(1000.0/kAnimRateInHz, [&]() { Tick(); }); }


void SplashController::Tick() {
	t_ += 1/kAnimRateInHz;
	if (t_ > kSplashDuration) {
		onComplete.Emit();
		return; }
	loop_.DrawScreenEventually();
	rclmt::Delay(1000.0/kAnimRateInHz, [&]() { Tick(); }); }


}  // namespace cxl
}  // namespace rqdq
