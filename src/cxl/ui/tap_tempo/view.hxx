#pragma once
#include <memory>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class TapTempoView : public TextKit::Widget {
public:
	TapTempoView(const int& taps);

	// Widget
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	int d_lastTaps{-1};
	const int& d_taps;
	rcls::TextCanvas d_canvas;
	bool d_dirty{true}; };


inline std::shared_ptr<TextKit::Widget> MakeTapTempoView(const int& taps) {
	return std::make_shared<TextKit::LineBox>(std::make_shared<TapTempoView>(taps)); }


}  // namespace cxl
}  // namespace rqdq
