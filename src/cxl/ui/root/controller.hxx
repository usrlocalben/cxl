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
	~RootController();
	RootController& operator=(const RootController& other) = delete;
	RootController(const RootController& other) = delete;

	// Widget impl
	bool HandleKeyEvent(TextKit::KeyEvent /*e*/) override;
	const rcls::TextCanvas& Draw(int w, int h) override {
		return view_.Draw(w, h); }
	std::pair<int, int> Pack(int w, int h) override {
		return view_.Pack(w, h); }
	int GetType() override {
		return view_.GetType(); }

	void Run();

private:
	void onLoaderStateChange();

	void IncrementMode();

private:
	rclmt::Event playbackStateChangedEvent_{rclmt::Event::MakeEvent()};
	TextKit::MainLoop loop_;
	class CXLUnit& unit_;
	class CXLASIOHost& host_;
	PatternController patternController_;
	LogController logController_;
	HostController hostController_;
	SplashController splashController_;
	RootView view_;
	int mode_{UM_PATTERN};

	int unitPlaybackStateChangedSignalId_{-1};
	int unitLoaderStateChangedSignalId_{-1};
	int splashCompleteSignalId_{-1}; };


}  // namespace cxl
}  // namespace rqdq
