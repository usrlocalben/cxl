#include "controller.hxx"

#include <memory>
#include <utility>

#include "src/cxl/host.hxx"
#include "src/cxl/ui/loading/view.hxx"
#include "src/cxl/ui/log/controller.hxx"
#include "src/cxl/ui/pattern/controller.hxx"
#include "src/cxl/ui/root/state.hxx"
#include "src/cxl/ui/root/view.hxx"
#include "src/cxl/ui/splash/controller.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_reactor_timer.hxx"
#include "src/rcl/rcls/rcls_console.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

RootController::RootController(CXLUnit& unit, CXLASIOHost& host)
	:d_unit(unit),
	d_host(host),
	d_patternController(unit, d_loop),
	d_logController(d_loop),
	d_hostController(d_host, d_loop),
	d_splashController(d_loop),
	d_view{d_unit, d_hostController.d_view, d_patternController.d_view, d_logController.d_view, d_mode} {

	auto& reactor = rclmt::Reactor::GetInstance();

	d_unit.ConnectPlaybackStateChanged([&](int playing) {
		d_playbackStateChangedEvent.Signal(); });
	reactor.ListenMany(d_playbackStateChangedEvent, [&]() {
		d_loop.DrawScreenEventually(); });

	d_unit.ConnectLoaderStateChanged([&]() {
		onLoaderStateChange(); });

	d_splashController.onComplete.connect([&]() {
		// can't reset() the Splash ptr while onComplete is firing
		// so queue this to run from the reactor
		rclmt::Delay(0, [&]() {
			d_view.d_splash.reset();
			d_loop.DrawScreenEventually(); }); });
	d_view.d_splash = d_splashController.d_view; }


void RootController::Run() {
	d_loop.DrawScreenEventually();
	d_loop.d_widget = this;
	d_loop.Run(); }


void RootController::IncrementMode() {
	d_mode = (d_mode+1)%(UI_MODES.size()); }


bool RootController::HandleKeyEvent(const TextKit::KeyEvent e) {
	using SC = rcls::ScanCode;
	auto& reactor = rclmt::Reactor::GetInstance();
	const auto key = e.scanCode;
	const auto down = e.down;
	if (down && e.control==rcls::kCKLeftAlt && key==SC::Q) {
		reactor.Stop();
		return true; }
	if (down && e.control==rcls::kCKLeftCtrl && key==SC::Enter) {
		IncrementMode();
		return true; }

	if (d_mode == UM_PATTERN) {
		return d_patternController.HandleKeyEvent(e); }
	return false; }


void RootController::onLoaderStateChange() {
	if (d_unit.IsLoading() && !d_view.d_loading) {
		d_view.d_loading = MakeLoadingView(d_unit); }
	else if (!d_unit.IsLoading() && d_view.d_loading) {
		d_view.d_loading.reset(); }
	d_loop.DrawScreenEventually(); }


}  // namespace cxl
}  // namespace rqdq
