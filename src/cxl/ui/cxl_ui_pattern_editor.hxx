#pragma once
#include "src/cxl/cxl_reactor.hxx"
#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/cxl_widget.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"

#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace rqdq {
namespace cxl {


class PatternEditor : public Widget {
public:
	PatternEditor(CXLUnit&);

	void onCXLUnitPlaybackPositionChangedMT(int);
	void onCXLUnitPlaybackPositionChanged();

	// Widget impl
	bool HandleKeyEvent(KEY_EVENT_RECORD) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	const rclw::ConsoleCanvas& DrawTrackSelection();
	const rclw::ConsoleCanvas& DrawParameters();
	const rclw::ConsoleCanvas& DrawGrid();
	const rclw::ConsoleCanvas& DrawPageIndicator();

private:
	const std::string GetPageParameterName(int pageNum, int trackNum, int paramNum);
	int GetPageParameterValue(int pageNum, int trackNum, int paramNum);
	void AdjustPageParameter(int pageNum, int trackNum, int paramNum, int offset);

private:
	void CopyTrackPage();
	void ClearTrackPage();
	void PasteTrackPage();

	WinEvent d_playbackPositionChangedEvent = WinEvent::MakeEvent();

	std::shared_ptr<Widget> d_popup;

	CXLUnit& d_unit;
	rclw::ConsoleCanvas d_canvas;
	int d_selectedTrack = 0;
	int d_selectedGridPage = 0;
	int d_selectedVoicePage = 0;
	std::array<int, 16> d_clipboard;
	bool d_isRecording = false; };


}  // namespace cxl
}  // namespace rqdq
