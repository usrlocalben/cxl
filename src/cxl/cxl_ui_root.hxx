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


class PatternLengthEdit : public Widget {
public:
	PatternLengthEdit(int value);

	// Widget
	bool HandleKeyEvent(KEY_EVENT_RECORD) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

	std::function<void(int)> onSuccess;
	std::function<void()> onCancel;
private:
	rclw::ConsoleCanvas d_canvas;
	bool d_dirty = true;
	int d_value = 0; };


class UIRoot : public Widget {
public:
	UIRoot(CXLUnit&);
	void OnCXLUnitChanged();
	void OnConsoleInputAvailable();

	void onCXLUnitPlaybackStateChangedMT(bool);
	void onCXLUnitPlaybackStateChanged();

	void onCXLUnitPlaybackPositionChangedMT(int);
	void onCXLUnitPlaybackPositionChanged();

	void onLogWrite();

	void onLoaderStateChange();

	// Widget impl
	bool HandleKeyEvent(KEY_EVENT_RECORD) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	const rclw::ConsoleCanvas& DrawHeader(int width);
	const rclw::ConsoleCanvas& DrawTrackSelection();
	const rclw::ConsoleCanvas& DrawParameters();
	const rclw::ConsoleCanvas& DrawGrid();
	const rclw::ConsoleCanvas& DrawPageIndicator();
	const rclw::ConsoleCanvas& DrawKeyHistory();
	const rclw::ConsoleCanvas& DrawLog();
	const rclw::ConsoleCanvas& DrawTransportIndicator(int width);

private:
	WindowsEvent d_playbackStateChangedEvent;
	WindowsEvent d_playbackPositionChangedEvent;
	void AddKeyDebuggerEvent(KEY_EVENT_RECORD);

	std::shared_ptr<Widget> d_popup;

	std::shared_ptr<Widget> d_loading;

	CXLUnit& d_unit;
	rclw::ConsoleCanvas d_canvas;
	bool d_editPatternLength = false;
	int d_selectedTrack = 0;
	int d_selectedGridPage = 0;
	int d_selectedParameterPage = 0;
	bool d_isRecording = false;
	bool d_enableKeyDebug = true;
	std::deque<std::string> d_keyHistory; };




}  // namespace cxl
}  // namespace rqdq
