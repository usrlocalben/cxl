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

	void onCXLUnitPlaybackPositionChangedASIO(int);
	void onCXLUnitPlaybackPositionChanged();

	bool HandleKeyEvent(TextKit::KeyEvent);

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
	rclmt::Event d_playbackPositionChangedEvent = rclmt::Event::MakeEvent();
	std::array<int, 16> d_clipboard; };


}  // namespace cxl
}  // namespace rqdq
