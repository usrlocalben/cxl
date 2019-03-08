#pragma once
#include "src/cxl/unit.hxx"
#include "src/cxl/ui/pattern_editor/view.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

#include <deque>
#include <memory>
#include <string>

namespace rqdq {
namespace cxl {


class UIRoot : public TextKit::Widget {
public:
	UIRoot(CXLUnit&);

	void Run();

	void onCXLUnitPlaybackStateChangedASIO(bool);
	void onCXLUnitPlaybackStateChanged();

	void onLogWrite();

	void onLoaderStateChange();

	// Widget impl
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	const rclw::ConsoleCanvas& DrawHeader(int width);
	const rclw::ConsoleCanvas& DrawKeyHistory();
	const rclw::ConsoleCanvas& DrawLog(int width, int height);
	const rclw::ConsoleCanvas& DrawTransportIndicator(int width);

private:
	rclmt::Event d_playbackStateChangedEvent = rclmt::Event::MakeEvent();
	// XXX void AddKeyDebuggerEvent(KEY_EVENT_RECORD);

	std::shared_ptr<TextKit::Widget> d_loading;
	PatternEditor d_patternEditor;

	CXLUnit& d_unit;
	TextKit::MainLoop d_loop;
	int d_mode = 0;
	rclw::ConsoleCanvas d_canvas;
	bool d_enableKeyDebug = true;
	std::deque<std::string> d_keyHistory; };


}  // namespace cxl
}  // namespace rqdq
