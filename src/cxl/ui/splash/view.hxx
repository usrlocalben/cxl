#pragma once
#include <functional>
#include <utility>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class SplashView : public TextKit::Widget {
public:
	SplashView(const float& t);

	void Invalidate();

	// Widget
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	const float& d_t;
	bool d_dirty{true};
	rcls::TextCanvas d_canvas; };


}  // namespace cxl
}  // namespace rqdq
