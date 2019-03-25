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
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*w*/, int /*h*/) override;
	int GetType() override;

private:
	bool Refresh();
	const int& d_valueSrc;

	int d_value{0};
	rcls::TextCanvas d_canvas; };


inline std::shared_ptr<TextKit::Widget> MakePatternLengthEditView(const int& value) {
	return std::make_shared<TextKit::LineBox>(std::make_shared<PatternLengthEditView>(value)); }


}  // namespace cxl
}  // namespace rqdq
