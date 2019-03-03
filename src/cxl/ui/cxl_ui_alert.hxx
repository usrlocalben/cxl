#pragma once
#include "src/cxl/cxl_widget.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"

#include <memory>

namespace rqdq {
namespace cxl {


class Alert : public Widget {
public:
	Alert(std::string text);

	// Widget
	bool HandleKeyEvent(KEY_EVENT_RECORD) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	rclw::ConsoleCanvas d_canvas;
	bool d_dirty = true;
	std::string d_text; };


inline std::shared_ptr<Widget> MakeAlert(const std::string& text) {
	return std::make_shared<LineBox>(std::make_shared<Alert>(text)); }


}  // namespace cxl
}  // namespace rqdq

