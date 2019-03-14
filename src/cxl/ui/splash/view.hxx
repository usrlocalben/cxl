#pragma once
#include <functional>
#include <utility>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

#include "wink/signal.hpp"

namespace rqdq {
namespace cxl {


class SplashView : public TextKit::Widget {
public:
	SplashView(TextKit::MainLoop& loop);

	// Widget
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

	wink::signal<std::function<void()>> onComplete;

private:
	void Tick();

private:
	float d_t{0.0f};
	bool d_dirty{true};
	TextKit::MainLoop& d_loop;
	rcls::TextCanvas d_canvas; };


}  // namespace cxl
}  // namespace rqdq
