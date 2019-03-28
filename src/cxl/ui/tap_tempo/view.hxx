#pragma once
#include <memory>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class TapTempoView : public TextKit::Widget {
public:
	explicit TapTempoView(const int& taps);

	// Widget
	bool HandleKeyEvent(TextKit::KeyEvent /*e*/) override;
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*w*/, int /*h*/) override;
	int GetType() override;

private:
	bool Refresh();
	const int& tapsSrc_;
	int taps_{-1};
	rcls::TextCanvas canvas_; };


inline std::shared_ptr<TextKit::Widget> MakeTapTempoView(const int& taps) {
	return std::make_shared<TextKit::LineBox>(std::make_shared<TapTempoView>(taps)); }


}  // namespace cxl
}  // namespace rqdq

