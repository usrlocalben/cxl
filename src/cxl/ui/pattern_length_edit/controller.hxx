#pragma once
#include <functional>

#include "src/cxl/ui/pattern_length_edit/view.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"

namespace rqdq {
namespace cxl {


class PatternLengthEditController {
public:
	PatternLengthEditController(int value);
	bool HandleKeyEvent(TextKit::KeyEvent);

	std::function<void(int)> onSuccess;
	std::function<void()> onCancel;

public:
	PatternLengthEditView d_view;

private:
	int d_value = 0; };


}  // namespace cxl
}  // namespace rqdq
