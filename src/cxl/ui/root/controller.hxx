#pragma once
#include <deque>
#include <memory>
// XXX #include <string>
#include <utility>

#include "src/cxl/host.hxx"
#include "src/cxl/ui/host/controller.hxx"
#include "src/cxl/ui/log/controller.hxx"
#include "src/cxl/ui/pattern/controller.hxx"
#include "src/cxl/ui/root/view.hxx"
#include "src/cxl/ui/splash/controller.hxx"
#include "src/cxl/unit.hxx"
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
	RootController(CXLUnit&, CXLASIOHost&);

	void Run();

	void onLoaderStateChange();

	// Widget impl
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rcls::TextCanvas& Draw(int w, int h) override {
		return d_view.Draw(w, h); }
	std::pair<int, int> Pack(int w, int h) override {
		return d_view.Pack(w, h); }
	int GetType() override {
		return d_view.GetType(); }

private:
	void IncrementMode();

private:
	rclmt::Event d_playbackStateChangedEvent = rclmt::Event::MakeEvent();
	// XXX void AddKeyDebuggerEvent(KEY_EVENT_RECORD);

	TextKit::MainLoop d_loop;
	CXLUnit& d_unit;
	CXLASIOHost& d_host;
	PatternController d_patternController;
	LogController d_logController;
	HostController d_hostController;
	SplashController d_splashController;

	RootView d_view;

	int d_mode = 0;
	// XXX bool d_enableKeyDebug = true;
	// XXX std::deque<std::string> d_keyHistory; };
	};


}  // namespace cxl
}  // namespace rqdq
