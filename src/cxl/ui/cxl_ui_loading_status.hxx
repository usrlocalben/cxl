#pragma once
#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/cxl_widget.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"

namespace rqdq {
namespace cxl {


class LoadingStatus : public Widget {
public:
	LoadingStatus(CXLUnit& unit);

	// Widget
	bool HandleKeyEvent(KEY_EVENT_RECORD) override;
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
