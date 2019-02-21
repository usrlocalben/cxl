#pragma once
#include "src/cxl/cxl_reactor.hxx"
#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/cxl_widget.hxx"
#include "src/rcl/rclw/rclw_console.hxx"

#include <deque>
#include <string>
#include <vector>

namespace rqdq {
namespace cxl {


class UIRoot : public Widget {
public:
	UIRoot(CXLUnit&);
	void OnCXLUnitChanged();
	void OnConsoleInputAvailable();

	void onCXLUnitPlaybackStateChangedMT(bool);
	void onCXLUnitPlaybackStateChanged();

	void onCXLUnitPlaybackPositionChangedMT(int);
	void onCXLUnitPlaybackPositionChanged();

	// Widget impl
	bool HandleKeyEvent(KEY_EVENT_RECORD);
	void Draw();
private:
	void DrawTrackSelection(int x, int y);
	void DrawParameters(int x, int y);
	void DrawGrid(int x, int y);
	void DrawKeyHistory(int x, int y);
	void DrawTransportIndicator();

private:
	WindowsEvent d_playbackStateChangedEvent;
	WindowsEvent d_playbackPositionChangedEvent;
	void AddKeyDebuggerEvent(KEY_EVENT_RECORD);

	CXLUnit& d_unit;
	int d_selectedTrack = 0;
	const int d_selectedPage = 0;
	bool d_enableKeyDebug = true;
	std::deque<std::string> d_keyHistory; };


}  // namespace cxl
}  // namespace rqdq
