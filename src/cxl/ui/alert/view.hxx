#pragma once
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

#include <memory>

namespace rqdq {
namespace cxl {


class Alert : public TextKit::Widget {
public:
	Alert(std::string text);

	// Widget
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	rclw::ConsoleCanvas d_canvas;
	bool d_dirty = true;
	std::string d_text; };


inline std::shared_ptr<TextKit::Widget> MakeAlert(const std::string& text) {
	return std::make_shared<TextKit::LineBox>(std::make_shared<Alert>(text)); }


}  // namespace cxl
}  // namespace rqdq

