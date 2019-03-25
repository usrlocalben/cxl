#pragma once
#include <array>
#include <memory>
#include <string>

#include "src/cxl/tap_tempo.hxx"
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
	PatternController(CXLUnit& /*unit*/, TextKit::MainLoop& loop);
	~PatternController();

	bool HandleKeyEvent(TextKit::KeyEvent /*e*/);
private:
	bool HandleKeyEvent2(TextKit::KeyEvent /*e*/);
	void KeyboardTick();
	void StartTick();
	void StopTick();

	void StartNudge(int dir);
	void StopNudge(int dir);

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
	TapTempo d_tapTempo{};
	int d_taps{0};
	int d_nudgeDir{0};
	int d_nudgeOldTempo{0};

	int d_timerId{-1};
	rclmt::Event d_playbackPositionChangedEvent{rclmt::Event::MakeEvent()};
	TextKit::KeyEvent d_prevKey{};
	std::array<int, 16> d_clipboard; };


}  // namespace cxl
}  // namespace rqdq
