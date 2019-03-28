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
	PatternController& operator=(const PatternController& other) = delete;
	PatternController(const PatternController& other) = delete;

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
	CXLUnit& unit_;
	TextKit::MainLoop& loop_;
	EditorState state_{};
    std::optional<PatternLengthEditController> patternLengthEditController_;
public:
	PatternView view_;

private:
	TapTempo tapTempo_{};
	int taps_{0};
	int nudgeDir_{0};
	int nudgeOldTempo_{0};

	int timerId_{-1};
	rclmt::Event playbackPositionChangedEvent_{rclmt::Event::MakeEvent()};
	TextKit::KeyEvent prevKey_{};
	std::array<int, 16> clipboard_;
	int signalId_{-1}; };


}  // namespace cxl
}  // namespace rqdq
