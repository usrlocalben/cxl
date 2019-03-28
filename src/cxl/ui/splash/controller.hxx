#pragma once
#include <functional>
#include <memory>

#include "src/rcl/rclmt/rclmt_signal.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class SplashController {
public:
	explicit SplashController(TextKit::MainLoop& loop);

	rclmt::Signal<void()> onComplete;

private:
	void Tick();

private:
	TextKit::MainLoop& loop_;
	float t_{0.0F};
public:
	std::shared_ptr<TextKit::Widget> view_; };


}  // namespace cxl
}  // namespace rqdq
