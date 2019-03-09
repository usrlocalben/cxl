#pragma once
#include <functional>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class PatternLengthEdit : public TextKit::Widget {
public:
	PatternLengthEdit(int value);

	// Widget
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

	std::function<void(int)> onSuccess;
	std::function<void()> onCancel;
private:
	rcls::TextCanvas d_canvas;
	bool d_dirty = true;
	int d_value = 0; };


}  // namespace cxl
}  // namespace rqdq
