#pragma once
#include <utility>

#include "src/cxl/ui/host/controller.hxx"
#include "src/cxl/ui/log/controller.hxx"
#include "src/cxl/ui/pattern/controller.hxx"
#include "src/cxl/ui/root/state.hxx"
#include "src/cxl/ui/root/view.hxx"
#include "src/cxl/ui/splash/controller.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {

class RootController : public TextKit::Widget {
public:
	RootController(class CXLUnit& /*unit*/, class CXLASIOHost& /*host*/);

	// Widget impl
	bool HandleKeyEvent(TextKit::KeyEvent /*e*/) override;
	const rcls::TextCanvas& Draw(int w, int h) override {
		return d_view.Draw(w, h); }
	std::pair<int, int> Pack(int w, int h) override {
		return d_view.Pack(w, h); }
	int GetType() override {
		return d_view.GetType(); }

	void Run();

private:
	void onLoaderStateChange();

	void IncrementMode();

private:
	rclmt::Event d_playbackStateChangedEvent{rclmt::Event::MakeEvent()};
	TextKit::MainLoop d_loop;
	class CXLUnit& d_unit;
	class CXLASIOHost& d_host;
	PatternController d_patternController;
	LogController d_logController;
	HostController d_hostController;
	SplashController d_splashController;
	RootView d_view;
	int d_mode{UM_PATTERN}; };


}  // namespace cxl
}  // namespace rqdq
