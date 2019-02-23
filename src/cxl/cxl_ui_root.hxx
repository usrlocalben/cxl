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


class PatternLengthEdit : public Widget {
public:
	PatternLengthEdit(int amt);

	// Widget
	bool HandleKeyEvent(KEY_EVENT_RECORD) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;

	std::function<void(int)> onSuccess;
	std::function<void()> onCancel;
private:
	int d_amt = 0; };


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
	const rclw::ConsoleCanvas& Draw(int, int) override;
private:
	const rclw::ConsoleCanvas& DrawHeader(int width);
	const rclw::ConsoleCanvas& DrawTrackSelection();
	const rclw::ConsoleCanvas& DrawParameters();
	const rclw::ConsoleCanvas& DrawGrid();
	const rclw::ConsoleCanvas& DrawPageIndicator();
	const rclw::ConsoleCanvas& DrawKeyHistory();
	const rclw::ConsoleCanvas& DrawTransportIndicator(int width);

private:
	WindowsEvent d_playbackStateChangedEvent;
	WindowsEvent d_playbackPositionChangedEvent;
	void AddKeyDebuggerEvent(KEY_EVENT_RECORD);

	std::unique_ptr<PatternLengthEdit> d_patternLengthEditor;

	CXLUnit& d_unit;
	bool d_editPatternLength = false;
	int d_selectedTrack = 0;
	int d_selectedGridPage = 0;
	int d_selectedParameterPage = 0;
	bool d_isRecording = false;
	bool d_enableKeyDebug = true;
	std::deque<std::string> d_keyHistory; };




}  // namespace cxl
}  // namespace rqdq
