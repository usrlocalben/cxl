#pragma once
#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/cxl_unit_view.hxx"
#include "src/rcl/rclw/rclw_console.hxx"

#include <deque>
#include <string>
#include <vector>

namespace rqdq {
namespace cxl {


class CXLUnitController {
public:
	CXLUnitController(rclw::Console&, CXLUnit&);
	void OnCXLUnitChanged();
	void OnConsoleInputAvailable();

private:
	void AddKeyDebuggerEvent(void*);

private:
	rclw::Console& d_console;
	CXLUnit& d_unit;
	int d_selectedTrack = 0;
	const int d_selectedPage = 0;
	bool d_enableKeyDebug = false;
	std::deque<std::string> d_keyHistory;
	std::vector<bool> d_downKeys; };


}  // namespace cxl
}  // namespace rqdq
