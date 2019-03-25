#pragma once
#include <functional>
#include <memory>

#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

#include "wink/signal.hpp"

namespace rqdq {
namespace cxl {


class SplashController {
public:
	SplashController(TextKit::MainLoop& loop);

	wink::signal<std::function<void()>> onComplete;

private:
	void Tick();

private:
	TextKit::MainLoop& d_loop;
	float d_t{0.0F};
public:
	std::shared_ptr<TextKit::Widget> d_view; };


}  // namespace cxl
}  // namespace rqdq
