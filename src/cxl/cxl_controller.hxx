#pragma once
#include "src/cxl/cxl_reactor.hxx"
#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/cxl_widget.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"

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
	bool HandleKeyEvent(KEY_EVENT_RECORD) override;
	rclw::ConsoleCanvas Draw() override;
private:
	rclw::ConsoleCanvas DrawHeader();
	rclw::ConsoleCanvas DrawTrackSelection();
	rclw::ConsoleCanvas DrawParameters();
	rclw::ConsoleCanvas DrawGrid();
	rclw::ConsoleCanvas DrawKeyHistory();
	rclw::ConsoleCanvas DrawTransportIndicator();

private:
	WindowsEvent d_playbackStateChangedEvent;
	WindowsEvent d_playbackPositionChangedEvent;
	void AddKeyDebuggerEvent(KEY_EVENT_RECORD);

	CXLUnit& d_unit;
	int d_selectedTrack = 0;
	const int d_selectedPage = 0;
	bool d_isRecording = false;
	bool d_enableKeyDebug = true;
	std::deque<std::string> d_keyHistory; };


}  // namespace cxl
}  // namespace rqdq
