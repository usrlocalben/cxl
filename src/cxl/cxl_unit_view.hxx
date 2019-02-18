#pragma once
#include "src/cxl/cxl_unit.hxx"
#include "src/rcl/rclw/rclw_console.hxx"

#include <array>
#include <deque>
#include <string>

namespace rqdq {
namespace cxl {


class CXLUnitView {
public:
	CXLUnitView(CXLUnit&, int, int, std::deque<std::string>&);
	void Draw(rclw::Console& console);

private:
	void DrawTrackSelection(rclw::Console& console, int x, int y);
	void DrawParameters(rclw::Console& console, int x, int y);
	void DrawGrid(rclw::Console& console, int x, int y);
	void DrawKeyHistory(rclw::Console& console, int x, int y);
	void DrawTransportIndicator(rclw::Console& console);

private:
	int d_selectedTrack = 0;
	int d_selectedPage = 0;
	std::deque<std::string>& d_keyHistory;
	CXLUnit& d_unit; };


}  // namespace cxl
}  // namespace rqdq
