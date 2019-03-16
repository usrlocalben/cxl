#include "src/cxl/ui/root/controller.hxx"

#include <memory>
#include <utility>

#include "src/cxl/host.hxx"
#include "src/cxl/ui/log/controller.hxx"
#include "src/cxl/ui/pattern/controller.hxx"
#include "src/cxl/ui/root/state.hxx"
#include "src/cxl/ui/root/view.hxx"
#include "src/cxl/ui/splash/controller.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_reactor_delay.hxx"
#include "src/rcl/rcls/rcls_console.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

using ScanCode = rcls::ScanCode;

RootController::RootController(CXLUnit& unit, CXLASIOHost& host)
	:d_unit(unit),
	d_host(host),
	d_patternController(unit, d_loop),
	d_logController(d_loop),
	d_hostController(d_host, d_loop),
	d_splashController(d_loop),
	d_loadingController(d_unit),
	d_view{d_unit, d_hostController.d_view, d_patternController.d_view, d_logController.d_view, d_mode} {

	auto& reactor = rclmt::Reactor::GetInstance();

	d_unit.d_playbackStateChanged.connect(this, &RootController::onCXLUnitPlaybackStateChangedASIO);
	reactor.ListenMany(d_playbackStateChangedEvent,
	                   [&]() { onCXLUnitPlaybackStateChanged(); });

	d_unit.d_loaderStateChanged.connect(this, &RootController::onLoaderStateChange);

	d_splashController.onComplete.connect([&]() {
		// can't reset() the Splash ptr while onComplete is firing
		// so queue this to run from the reactor
		rclmt::Delay(0, [&]() {
			d_view.d_splash.reset();
			d_loop.DrawScreenEventually(); }); });
	d_view.d_splash = std::make_shared<TextKit::LineBox>(&d_splashController.d_view); }


void RootController::Run() {
	d_loop.DrawScreenEventually();
	d_loop.d_widget = this;
	d_loop.Run(); }


bool RootController::HandleKeyEvent(const TextKit::KeyEvent e) {
	// XXX AddKeyDebuggerEvent(e);
	auto& reactor = rclmt::Reactor::GetInstance();
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::Q) {
		reactor.Stop();
		return true; }
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::Enter) {
		if (d_mode == UM_PATTERN) {
			d_mode = UM_LOG; }
		else if (d_mode == UM_LOG) {
			d_mode = UM_HOST; }
		else if (d_mode == UM_HOST) {
			d_mode = UM_PATTERN; }
		return true; }

	if (d_mode == UM_PATTERN) {
		return d_patternController.HandleKeyEvent(e); }
	return false; }


void RootController::onCXLUnitPlaybackStateChangedASIO(bool isPlaying) {
	// this is called from the ASIO thread.
	// use a Reactor event to bounce to main
	d_playbackStateChangedEvent.Signal(); }
void RootController::onCXLUnitPlaybackStateChanged() {
	d_loop.DrawScreenEventually(); }


void RootController::onLoaderStateChange() {
	if (d_unit.IsLoading() && !d_view.d_loading) {
		d_view.d_loading = std::make_shared<TextKit::LineBox>(&d_loadingController.d_view); }
	else if (!d_unit.IsLoading() && d_view.d_loading) {
		d_view.d_loading.reset(); }
	d_loop.DrawScreenEventually(); }

/*
XXX
void RootController::AddKeyDebuggerEvent(KEY_EVENT_RECORD e) {
	if (d_enableKeyDebug) {
		auto s = fmt::sprintf("%c %c % 3d",
		                      (e.bKeyDown != 0?'D':'U'),
		                      //e.control,
		                      e.uChar.AsciiChar,
		                      //e.wRepeatCount,
		                      //e.scanCode,
		                      e.scanCode);
		d_keyHistory.emplace_back(s);
		if (d_keyHistory.size() > 8) {
			d_keyHistory.pop_front(); }}}*/


}  // namespace cxl
}  // namespace rqdq
