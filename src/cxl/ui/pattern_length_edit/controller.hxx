#pragma once
#include <functional>
#include <memory>

#include "src/cxl/ui/pattern_length_edit/view.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class PatternLengthEditController {
public:
	explicit PatternLengthEditController(int value);
	bool HandleKeyEvent(TextKit::KeyEvent /*e*/);

	std::function<void(int)> onSuccess;
	std::function<void()> onCancel;

public:
	std::shared_ptr<TextKit::Widget> view_;

private:
	int value_ = 0; };


}  // namespace cxl
}  // namespace rqdq
