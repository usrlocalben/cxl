#pragma once
#include <deque>
#include <memory>
#include <string>

#include "src/cxl/ui/pattern_editor/view.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

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
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	const rcls::TextCanvas& DrawHeader(int width);
	const rcls::TextCanvas& DrawKeyHistory();
	const rcls::TextCanvas& DrawLog(int width, int height);
	const rcls::TextCanvas& DrawTransportIndicator(int width);

private:
	rclmt::Event d_playbackStateChangedEvent = rclmt::Event::MakeEvent();
	// XXX void AddKeyDebuggerEvent(KEY_EVENT_RECORD);

	std::shared_ptr<TextKit::Widget> d_loading;
	PatternEditor d_patternEditor;

	CXLUnit& d_unit;
	TextKit::MainLoop d_loop;
	int d_mode = 0;
	rcls::TextCanvas d_canvas;
	bool d_enableKeyDebug = true;
	std::deque<std::string> d_keyHistory; };


}  // namespace cxl
}  // namespace rqdq
