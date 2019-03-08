#pragma once
#include "src/cxl/unit.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class LoadingStatus : public TextKit::Widget {
public:
	LoadingStatus(CXLUnit& unit);

	// Widget
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;
private:
	void onLoaderStateChange();

private:
	rclw::ConsoleCanvas d_canvas;
	CXLUnit& d_unit;
	bool d_dirty = true; };


}  // namespace cxl
}  // namespace rqdq
