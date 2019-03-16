#pragma once
#include <functional>

#include "src/cxl/ui/splash/view.hxx"
#include "src/textkit/mainloop.hxx"

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
	float d_t{0.0f};
public:
	SplashView d_view; };


}  // namespace cxl
}  // namespace rqdq
