#pragma once
#include "src/textkit/widget.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

#include <functional>

namespace rqdq {
namespace cxl {


class PatternLengthEdit : public TextKit::Widget {
public:
	PatternLengthEdit(int value);

	// Widget
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

	std::function<void(int)> onSuccess;
	std::function<void()> onCancel;
private:
	rclw::ConsoleCanvas d_canvas;
	bool d_dirty = true;
	int d_value = 0; };


}  // namespace cxl
}  // namespace rqdq

