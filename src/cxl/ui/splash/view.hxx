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
	bool Refresh();
	const float& d_tSrc;
	float d_t{0};
	rcls::TextCanvas d_canvas; };


inline std::shared_ptr<TextKit::Widget> MakeSplashView(const float& t) {
	return std::make_shared<TextKit::LineBox>(std::make_shared<SplashView>(t)); }


}  // namespace cxl
}  // namespace rqdq
