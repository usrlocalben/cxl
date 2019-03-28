#include "controller.hxx"

#include <memory>
#include <utility>

#include "src/cxl/host.hxx"
#include "src/cxl/log.hxx"
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
	:unit_(unit),
	host_(host),
	patternController_(unit, loop_),
	logController_(loop_),
	hostController_(host_, loop_),
	splashController_(loop_),
	view_{unit_, hostController_.view_, patternController_.view_, logController_.view_, mode_} {

	auto& reactor = rclmt::Reactor::GetInstance();

	unitPlaybackStateChangedSignalId_ = unit_.ConnectPlaybackStateChanged([&](int playing) {
		playbackStateChangedEvent_.Signal(); });
	reactor.ListenMany(playbackStateChangedEvent_, [&]() {
		loop_.DrawScreenEventually(); });

	unitLoaderStateChangedSignalId_ = unit_.ConnectLoaderStateChanged([&]() {
		onLoaderStateChange(); });

	splashCompleteSignalId_ = splashController_.onComplete.Connect([&]() {
		// can't reset() the Splash ptr while onComplete is firing
		// so queue this to run from the reactor
		rclmt::Delay(0, [&]() {
			view_.splash_.reset();
			loop_.DrawScreenEventually(); }); });
	view_.splash_ = splashController_.view_; }


void RootController::Run() {
	loop_.DrawScreenEventually();
	loop_.d_widget = this;
	loop_.Run(); }


void RootController::IncrementMode() {
	mode_ = (mode_+1)%(UI_MODES.size()); }


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

	if (mode_ == UM_PATTERN) {
		return patternController_.HandleKeyEvent(e); }
	return false; }


void RootController::onLoaderStateChange() {
	auto& log = Log::GetInstance();
	if (unit_.IsLoading()) {
		if (!view_.loading_) {
			view_.loading_ = MakeLoadingView(unit_); }}
	else {
		if (view_.loading_) {
			view_.loading_.reset(); }}
	loop_.DrawScreenEventually(); }


RootController::~RootController() {
	unit_.DisconnectPlaybackStateChanged(unitPlaybackStateChangedSignalId_);
	unit_.DisconnectLoaderStateChanged(unitLoaderStateChangedSignalId_);
	splashController_.onComplete.Disconnect(splashCompleteSignalId_); }


}  // namespace cxl
}  // namespace rqdq
