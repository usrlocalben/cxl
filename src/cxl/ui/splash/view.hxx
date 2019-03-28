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
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*w*/, int /*h*/) override;
	int GetType() override;

private:
	bool Refresh();
	const float& tSrc_;
	float t_{0};
	rcls::TextCanvas canvas_; };


inline std::shared_ptr<TextKit::Widget> MakeSplashView(const float& t) {
	return std::make_shared<TextKit::LineBox>(std::make_shared<SplashView>(t)); }


}  // namespace cxl
}  // namespace rqdq
