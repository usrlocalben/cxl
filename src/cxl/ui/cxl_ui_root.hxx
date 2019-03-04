#pragma once
#include "src/cxl/cxl_reactor.hxx"
#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/cxl_widget.hxx"
#include "src/cxl/ui/cxl_ui_pattern_editor.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"

#include <deque>
#include <memory>
#include <string>

namespace rqdq {
namespace cxl {


class UIRoot : public Widget {
public:
	UIRoot(CXLUnit&);

	void onCXLUnitPlaybackStateChangedMT(bool);
	void onCXLUnitPlaybackStateChanged();

	void onLogWrite();

	void onLoaderStateChange();

	// Widget impl
	bool HandleKeyEvent(KEY_EVENT_RECORD) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	const rclw::ConsoleCanvas& DrawHeader(int width);
	const rclw::ConsoleCanvas& DrawKeyHistory();
	const rclw::ConsoleCanvas& DrawLog(int width, int height);
	const rclw::ConsoleCanvas& DrawTransportIndicator(int width);

private:
	WinEvent d_playbackStateChangedEvent = WinEvent::MakeEvent();
	void AddKeyDebuggerEvent(KEY_EVENT_RECORD);

	std::shared_ptr<Widget> d_loading;
	PatternEditor d_patternEditor;

	CXLUnit& d_unit;
	int d_mode = 0;
	rclw::ConsoleCanvas d_canvas;
	bool d_enableKeyDebug = true;
	std::deque<std::string> d_keyHistory; };


}  // namespace cxl
}  // namespace rqdq
