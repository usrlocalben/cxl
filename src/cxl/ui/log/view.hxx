#pragma once
#include <utility>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {

class LogView : public TextKit::Widget {
public:
	LogView();

	// Widget impl
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*w*/, int /*h*/) override;
	int GetType() override;

private:
	rcls::TextCanvas canvas_; };


}  // namespace cxl
}  // namespace rqdq
