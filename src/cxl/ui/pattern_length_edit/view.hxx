#pragma once
#include <utility>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class PatternLengthEditView : public TextKit::Widget {
public:
	PatternLengthEditView(const int& value);

	// Widget
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

	void Invalidate();

private:
	const int& d_value;

	rcls::TextCanvas d_canvas;
	bool d_dirty{true}; };


}  // namespace cxl
}  // namespace rqdq
