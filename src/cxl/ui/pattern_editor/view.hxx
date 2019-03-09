#pragma once
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "src/cxl/unit.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {

class PatternEditor : public TextKit::Widget {
public:
	PatternEditor(CXLUnit&, TextKit::MainLoop& loop);

	void onCXLUnitPlaybackPositionChangedASIO(int);
	void onCXLUnitPlaybackPositionChanged();

	// Widget impl
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	const rcls::TextCanvas& DrawTrackSelection();
	const rcls::TextCanvas& DrawParameters();
	const rcls::TextCanvas& DrawGrid();
	const rcls::TextCanvas& DrawPageIndicator();

private:
	const std::string GetPageParameterName(int pageNum, int trackNum, int paramNum);
	int GetPageParameterValue(int pageNum, int trackNum, int paramNum);
	void AdjustPageParameter(int pageNum, int trackNum, int paramNum, int offset);

private:
	void CopyTrackPage();
	void ClearTrackPage();
	void PasteTrackPage();

	rclmt::Event d_playbackPositionChangedEvent = rclmt::Event::MakeEvent();

	std::shared_ptr<TextKit::Widget> d_popup;

	CXLUnit& d_unit;
	TextKit::MainLoop& d_loop;
	rcls::TextCanvas d_canvas;
	int d_selectedTrack = 0;
	int d_selectedGridPage = 0;
	int d_selectedVoicePage = 0;
	std::array<int, 16> d_clipboard;
	bool d_isRecording = false; };


}  // namespace cxl
}  // namespace rqdq
