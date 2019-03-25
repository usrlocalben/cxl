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
	SplashController(TextKit::MainLoop& loop);

	rclmt::Signal<void()> onComplete;

private:
	void Tick();

private:
	TextKit::MainLoop& d_loop;
	float d_t{0.0F};
public:
	std::shared_ptr<TextKit::Widget> d_view; };


}  // namespace cxl
}  // namespace rqdq
