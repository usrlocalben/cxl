#include "src/cxl/ui/cxl_ui_root.hxx"
#include "src/cxl/cxl_log.hxx"
#include "src/cxl/cxl_reactor.hxx"
#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/ui/cxl_ui_loading_status.hxx"
#include "src/cxl/ui/cxl_ui_pattern_editor.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"

#include <array>
#include <deque>
#include <sstream>
#include <string>
#include <vector>
#include <Windows.h>
#include "3rdparty/fmt/include/fmt/printf.h"

namespace rqdq {

namespace {

constexpr int UM_PATTERN = 0;
constexpr int UM_LOG = 1;

}  // namespace

namespace cxl {

using ScanCode = rclw::ScanCode;


UIRoot::UIRoot(CXLUnit& unit)
	:d_unit(unit), d_patternEditor(unit) {
	auto& reactor = Reactor::GetInstance();

	d_unit.d_playbackStateChanged.connect(this, &UIRoot::onCXLUnitPlaybackStateChangedMT);
	reactor.ListenForever({ d_playbackStateChangedEvent.GetHandle(),
	                        [&]() { onCXLUnitPlaybackStateChanged(); }});

	d_unit.d_loaderStateChanged.connect(this, &UIRoot::onLoaderStateChange);

	d_loading = std::make_shared<LineBox>(
		std::make_shared<LoadingStatus>(d_unit)
		);

	auto& log = Log::GetInstance();
	log.d_updated.connect([&]() { onLogWrite(); }); }


std::pair<int, int> UIRoot::Pack(int w, int h) {
	return {80, 25}; }


int UIRoot::GetType() {
	return WT_BOX; }


const rclw::ConsoleCanvas& UIRoot::Draw(int width, int height) {
	auto& out = d_canvas;
	out.Resize(width, height);
	out.Clear();
	WriteXY(out, 0, 0, DrawHeader(width));

	//if (d_enableKeyDebug) {
	//	WriteXY(out, width-20, height-15, DrawKeyHistory()); }

	WriteXY(out, 0, height-1, DrawTransportIndicator(width));

	if (d_mode == UM_PATTERN) {
		const auto& overlay = d_patternEditor.Draw(width, height-2);
		WriteXY(out, 0, 1, overlay); }
	else if (d_mode == UM_LOG) {
		const auto& overlay = DrawLog(width, height-2);
		WriteXY(out, 0, 1, overlay); }

	if (d_unit.IsLoading()) {
		auto attr = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlack);
		Fill(out, attr);
		auto [sx, sy] = d_loading->Pack(-1, -1);
		const auto& overlay = d_loading->Draw(sx, sy);
		int xc = (width - overlay.d_width) / 2;
		int yc = (height - overlay.d_height) / 2;
		WriteXY(out, xc, yc, overlay); }

	return out; }


const rclw::ConsoleCanvas& UIRoot::DrawLog(int width, int height) {
	auto& log = Log::GetInstance();
	static rclw::ConsoleCanvas out;
	out.Resize(width, height);
	out.Clear();
	Fill(out, rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlack));
	int head = log.GetHeadIdx();
	for (int y=0; y<height; y++) {
		WriteXY(out, 0, y, log.GetEntry(height-y-1, head)); }
	return out; }


const rclw::ConsoleCanvas& UIRoot::DrawHeader(int width) {
	static rclw::ConsoleCanvas out(width, 1);
	out.Resize(width, 1);
	out.Clear();
	Fill(out, rclw::MakeAttribute(rclw::Color::Red, rclw::Color::Black));
	WriteXY(out, 1, 0, "cxl 0.1.0");
	WriteXY(out, width-9-1, 0, "anix/rqdq");
	return out; }


const rclw::ConsoleCanvas& UIRoot::DrawKeyHistory() {
	static rclw::ConsoleCanvas out{ 10, 8 };
	out.Clear();
	Fill(out, rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Blue));
	int row = 0;
	for (const auto& item : d_keyHistory) {
		WriteXY(out, 0, row++, item); }
	return out; }


const rclw::ConsoleCanvas& UIRoot::DrawTransportIndicator(int width) {
	static rclw::ConsoleCanvas out{ width, 1 };
	out.Clear();
	auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Blue);
	auto hi = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlue);
	auto higreen = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongGreen);
	// auto hired = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongRed);

	//Fill(out, rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlue));
	std::string s;
	WriteXY(out, 1, 0, "Pattern: ", lo);
	WriteXY(out, 10, 0, fmt::sprintf("A-%d", d_unit.GetCurrentPatternNumber()+1), hi);

	WriteXY(out, 14, 0, "| Kit: ", lo);
	WriteXY(out, 21, 0, fmt::sprintf("%d", d_unit.GetCurrentKitNumber()+1), hi);

	int tempo = d_unit.GetTempo();
	int whole = tempo/10;
	int tenths = tempo%10;

	WriteXY(out, width-33, 0, "Tempo:", lo);
	s = fmt::sprintf("%3d.%d bpm", whole, tenths);
	WriteXY(out, width-26, 0, s, hi);
	WriteXY(out, width-16, 0, "|", lo);
	WriteXY(out, width-14, 0, d_unit.IsPlaying() ? "PLAYING" : "STOPPED", d_unit.IsPlaying() ? higreen : lo);
	WriteXY(out, width-6, 0, "|", lo);
	// XXX WriteXY(out, width-4, 0, d_isRecording ? "REC" : "rec", d_isRecording ? hired : lo);
	return out; }


bool UIRoot::HandleKeyEvent(const KEY_EVENT_RECORD e) {
	AddKeyDebuggerEvent(e);
	auto& reactor = Reactor::GetInstance();
	if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && e.wVirtualScanCode==ScanCode::Q) {
		reactor.Stop();
		return true; }
	if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && e.wVirtualScanCode==ScanCode::Enter) {
		if (d_mode == UM_PATTERN) {
			d_mode = UM_LOG; }
		else if (d_mode == UM_LOG) {
			d_mode = UM_PATTERN; }
		return true; }

	if (d_mode == UM_PATTERN) {
		return d_patternEditor.HandleKeyEvent(e); }
	return false; }


void UIRoot::onCXLUnitPlaybackStateChangedMT(bool isPlaying) {
	// this is called from the ASIO thread.
	// use a Reactor event to bounce to main
	d_playbackStateChangedEvent.Set(); }
void UIRoot::onCXLUnitPlaybackStateChanged() {
	Reactor::GetInstance().DrawScreenEventually(); }


void UIRoot::onLogWrite() {
	Reactor::GetInstance().DrawScreenEventually(); }


void UIRoot::onLoaderStateChange() {
	Reactor::GetInstance().DrawScreenEventually(); }


void UIRoot::AddKeyDebuggerEvent(KEY_EVENT_RECORD e) {
	if (d_enableKeyDebug) {
		auto s = fmt::sprintf("%c %c % 3d",
		                      (e.bKeyDown != 0?'D':'U'),
		                      //e.dwControlKeyState,
		                      e.uChar.AsciiChar,
		                      //e.wRepeatCount,
		                      //e.wVirtualKeyCode,
		                      e.wVirtualScanCode);
		d_keyHistory.emplace_back(s);
		if (d_keyHistory.size() > 8) {
			d_keyHistory.pop_front(); }}}


}  // namespace cxl
}  // namespace rqdq
