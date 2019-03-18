#pragma once
#include <array>
#include <memory>
#include <string>

#include "src/cxl/ui/pattern/state.hxx"
#include "src/cxl/ui/pattern/view.hxx"
#include "src/cxl/ui/pattern_length_edit/controller.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

class PatternController {
public:
	PatternController(CXLUnit&, TextKit::MainLoop& loop);
	~PatternController();

	void onCXLUnitPlaybackPositionChangedASIO(int);
	void onCXLUnitPlaybackPositionChanged();

	bool HandleKeyEvent(TextKit::KeyEvent);
private:
	bool HandleKeyEvent2(TextKit::KeyEvent);
	void KeyboardTick();
	void StartTick();
	void StopTick();

private:
	void CopyTrackPage();
	void ClearTrackPage();
	void PasteTrackPage();

private:
	CXLUnit& d_unit;
	TextKit::MainLoop& d_loop;
	EditorState d_state{};
    std::optional<PatternLengthEditController> d_patternLengthEditController;
public:
	PatternView d_view;

private:
	int d_timerId = -1;
	rclmt::Event d_playbackPositionChangedEvent = rclmt::Event::MakeEvent();
	TextKit::KeyEvent d_prevKey{};
	std::array<int, 16> d_clipboard; };


}  // namespace cxl
}  // namespace rqdq
