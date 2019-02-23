#include "src/cxl/cxl_controller.hxx"

#include "src/cxl/cxl_reactor.hxx"
#include "src/cxl/cxl_unit.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"

#include <array>
#include <deque>
#include <sstream>
#include <string>
#include <vector>

#define NOMINMAX
#include <Windows.h>
#include "3rdparty/fmt/include/fmt/printf.h"

namespace rqdq {

namespace {

using SC = rclw::ScanCode;

int pos2page(int pos) {
	if (pos < 16) return 0;
	if (pos < 32) return 1;
	if (pos < 48) return 2;
	if (pos < 64) return 3;
	auto msg = fmt::sprintf("unexpected pos %d", pos);
	throw std::runtime_error(msg); }



/**
 * LUT for converting a 4x4 matrix of scan-codes to
 * indices for grid positions, track#, pattern# etc
 */
constexpr std::array<char, 16> kGridScanLUT = {
	SC::Key1, SC::Key2, SC::Key3, SC::Key4,
	SC::Q,    SC::W,    SC::E,    SC::R,
	SC::A,    SC::S,    SC::D,    SC::F,
	SC::Z,    SC::X,    SC::C,    SC::V };

/**
 * LUT for converting a 4x4 matrix of scan-codes to
 * indices for parameter controls
 */
constexpr std::array<char, 8> kParamScanLUT = {
	SC::T, SC::Y, SC::U, SC::I,
	SC::G, SC::H, SC::J, SC::K, };


const std::array<const std::string, 16> kTrackNames = {
	"BD", "SD", "HT", "MD", "LT", "CP", "RS", "CB",
	"CH", "OH", "RC", "CC", "M1", "M2", "M3", "M4" };


char tolower(char ch) {
	if ('A' <= ch && ch <= 'Z') {
		return ch - 'A' + 'a'; }
	return ch; }


const std::string& tolower(const std::string& s) {
	thread_local std::string tmp;
	tmp.clear();
	for (auto ch : s) {
		tmp.push_back(tolower(ch)); }
	return tmp; }

}  // namespace

namespace cxl {

using ScanCode = rclw::ScanCode;

UIRoot::UIRoot(CXLUnit& unit)
	:d_unit(unit) {
	auto& reactor = Reactor::GetInstance();

	d_unit.d_playbackStateChanged.connect(this, &UIRoot::onCXLUnitPlaybackStateChangedMT);
	reactor.AddEvent({ d_playbackStateChangedEvent.GetHandle(),
	                   [&]() { onCXLUnitPlaybackStateChanged(); }});

	d_unit.d_playbackPositionChanged.connect(this, &UIRoot::onCXLUnitPlaybackPositionChangedMT);
	reactor.AddEvent({ d_playbackPositionChangedEvent.GetHandle(),
	                   [&]() { onCXLUnitPlaybackPositionChanged(); }}); }


/**
 * the playback signal handlers are called from the ASIO
 * thread.  set the associated event, and it will be
 * handled by the main thread.
 */
void UIRoot::onCXLUnitPlaybackStateChangedMT(bool isPlaying) {
	d_playbackStateChangedEvent.Set(); }
void UIRoot::onCXLUnitPlaybackStateChanged() {
	Reactor::GetInstance().DrawScreen(); }


void UIRoot::onCXLUnitPlaybackPositionChangedMT(int pos) {
	d_playbackPositionChangedEvent.Set(); }
void UIRoot::onCXLUnitPlaybackPositionChanged() {
	Reactor::GetInstance().DrawScreen(); }


rclw::ConsoleCanvas UIRoot::Draw() {
	rclw::ConsoleCanvas tmp(80, 25);
	tmp = Flatten(tmp, DrawHeader(), 0, 0);
	tmp = Flatten(tmp, DrawTrackSelection(), 1, 2);
	tmp = Flatten(tmp, DrawGrid(), 1, 21);
	tmp = Flatten(tmp, DrawPageIndicator(), 68, 22);
	if (d_enableKeyDebug) {
		tmp = Flatten(tmp, DrawKeyHistory(), 60, 10); }
	tmp = Flatten(tmp, DrawParameters(), 8, 6);
	tmp = Flatten(tmp, DrawTransportIndicator(), 0, 24);
	return tmp; }


rclw::ConsoleCanvas UIRoot::DrawHeader() {
	rclw::ConsoleCanvas out(80, 1);
	Fill(out, rclw::MakeAttribute(rclw::Color::Red, rclw::Color::Black));
	WriteXY(out, 1, 0, "cxl 0.1.0");
	WriteXY(out, 80-9-1, 0, "anix/rqdq");
	return out; }


rclw::ConsoleCanvas UIRoot::DrawTrackSelection() {
	rclw::ConsoleCanvas out{ 25, 2 };
	auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Cyan);
	auto hi = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongCyan);
	for (int i = 0; i < 8; i++) {
		WriteXY(out, i*3+1, 0, tolower(kTrackNames[i]), lo); }
	for (int i = 8; i <16; i++) {
		WriteXY(out, (i-8)*3+1, 1, tolower(kTrackNames[i]), lo); }

	int selY = d_selectedTrack / 8;
	int selX = (d_selectedTrack % 8) * 3 + 1;
	WriteXY(out, selX-1, selY, "[" + kTrackNames[d_selectedTrack] + "]", hi);
	return out; }


rclw::ConsoleCanvas UIRoot::DrawParameters() {
	rclw::ConsoleCanvas out{ 30, 8 };
	Fill(out, rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Cyan));
	for (int i = 0; i < 8; i++) {
		int paramNum = d_selectedParameterPage*8+i;
		auto& paramName = d_unit.GetVoiceParameterName(d_selectedTrack, paramNum);
		int value = d_unit.GetVoiceParameterValue(d_selectedTrack, paramNum);
		WriteXY(out, 0, i, fmt::sprintf("% 12s : % 3d", paramName, value)); }
	int waveId = d_unit.GetVoiceParameterValue(d_selectedTrack, 4);
	std::string waveName = d_unit.GetWaveName(waveId);
	WriteXY(out, 19, 4, waveName);
	return out; }


rclw::ConsoleCanvas UIRoot::DrawGrid() {
	rclw::ConsoleCanvas out{ 65, 2 };
	auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Brown);
	auto hi = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBrown);
	auto red = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongRed);
	Fill(out, lo);
	WriteXY(out, 0, 0, "| .   .   .   . | .   .   .   . | .   .   .   . | .   .   .   . | ");
	const int curPos = d_unit.GetLastPlayedGridPosition();
	const int curPage = pos2page(curPos);

	if (curPage == d_selectedGridPage) {
		WriteXY(out, 2+curPos*4, 0, "o", hi); }

	WriteXY(out, 0, 1, "|");
	for (int i = 0; i < 16; i++) {
		auto value = d_unit.GetTrackGridNote(d_selectedTrack, d_selectedGridPage*16+i);
		WriteXY(out, i*4+2, 1, value != 0 ? "X" : " ", red);
		WriteXY(out, i*4+4, 1, "|", lo); }
	return out; }


rclw::ConsoleCanvas UIRoot::DrawPageIndicator() {
	rclw::ConsoleCanvas out{ 9, 1 };
	int numPages = d_unit.GetPatternLength() / 16;
	int curPage = d_selectedGridPage;
	int playingPage = pos2page(d_unit.GetLastPlayedGridPosition());

	auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Brown);
	auto hi = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBrown);
	auto red = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongRed);
	Fill(out, lo);
	WriteXY(out, 0, 0, "[. . . .]", lo);
	const std::array<int,4> xa = { 1, 3, 5, 7 };
	
	for (int n=0; n<4; n++) {
	if (d_unit.IsPlaying() && playingPage == n) {
		WriteXY(out, xa[n], 0, "o", red); }
	else if (curPage == n) {
		WriteXY(out, xa[n], 0, "o", hi); }
	else {}}

	return out; }


rclw::ConsoleCanvas UIRoot::DrawKeyHistory() {
	rclw::ConsoleCanvas out{ 10, 8 };
	Fill(out, rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Blue));
	int row = 0;
	for (const auto& item : d_keyHistory) {
		WriteXY(out, 0, row++, item); }
	return out; }


rclw::ConsoleCanvas UIRoot::DrawTransportIndicator() {
	rclw::ConsoleCanvas out{ 80, 1 };
	auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Blue);
	auto hi = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlue);
	auto higreen = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongGreen);
	auto hired = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongRed);

	//Fill(out, rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlue));
	std::string s;
	WriteXY(out, 1, 0, "Pattern: ", lo);
	WriteXY(out, 10, 0, fmt::sprintf("A-%d", d_unit.GetCurrentPatternNumber()+1), hi);

	WriteXY(out, 14, 0, "| Kit: ", lo);
	WriteXY(out, 21, 0, fmt::sprintf("%d", d_unit.GetCurrentKitNumber()+1), hi);

	int tempo = d_unit.GetTempo();
	int whole = tempo/10;
	int tenths = tempo%10;
	WriteXY(out, 47, 0, "Tempo:", lo);
	s = fmt::sprintf("%3d.%d bpm", whole, tenths);
	WriteXY(out, 54, 0, s, hi);
	WriteXY(out, 64, 0, "|", lo);
	WriteXY(out, 66, 0, d_unit.IsPlaying() ? "PLAYING" : "STOPPED", d_unit.IsPlaying() ? higreen : lo);
	WriteXY(out, 74, 0, "|", lo);
	WriteXY(out, 76, 0, d_isRecording ? "REC" : "rec", d_isRecording ? hired : lo);
	return out; }


bool UIRoot::HandleKeyEvent(const KEY_EVENT_RECORD e) {
	AddKeyDebuggerEvent(e);
	auto& reactor = Reactor::GetInstance();
	if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && e.wVirtualScanCode==ScanCode::Q) {
		reactor.Stop();
		return true; }
	if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && (ScanCode::Key1<=e.wVirtualScanCode && e.wVirtualScanCode<=ScanCode::Key8)) {
		// Ctrl+1...Ctrl+8
		d_selectedTrack = e.wVirtualScanCode - ScanCode::Key1;
		return true; }
	// xxx if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCK
	if ((e.bKeyDown != 0) && e.dwControlKeyState==0) {
		if (e.wVirtualScanCode == ScanCode::Backslash) {
			d_selectedGridPage++;
			if (d_selectedGridPage >= d_unit.GetPatternLength()/16) {
				d_selectedGridPage = 0; }
			return true; }
		if (e.wVirtualScanCode == ScanCode::L) { d_isRecording = !d_isRecording; d_unit.CommitPattern(); return true; }
		if (e.wVirtualScanCode == ScanCode::Semicolon) { d_unit.Play(); return true; }
		if (e.wVirtualScanCode == ScanCode::Quote) { d_unit.Stop(); return true; }
		if (e.wVirtualScanCode == ScanCode::F5) { d_unit.SaveKit(); return true; }
		if (e.wVirtualScanCode == ScanCode::F6) { d_unit.LoadKit(); return true; }
		if (e.wVirtualScanCode == ScanCode::F7) { d_unit.DecrementKit(); return true; }
		if (e.wVirtualScanCode == ScanCode::F8) { d_unit.IncrementKit(); return true; }
		if (e.wVirtualScanCode == ScanCode::Comma || e.wVirtualScanCode == ScanCode::Period) {

			int amt = (e.wVirtualScanCode == ScanCode::Comma ? -1 : 1);
			if ((e.dwControlKeyState & rclw::kCKShift) != 0u) {
				// XXX does not work because of MSFT internal bug 9311951
				// https://github.com/Microsoft/WSL/issues/1188
				amt *= 10; }

			auto it = std::find_if(begin(kParamScanLUT), end(kParamScanLUT),
								   [&](auto& item) { return reactor.GetKeyState(item); });
			if (it != end(kParamScanLUT)) {
				int idx = std::distance(begin(kParamScanLUT), it);
				d_unit.Adjust(d_selectedTrack, d_selectedParameterPage*8+idx, amt); }
			else if (reactor.GetKeyState(ScanCode::Equals)) {
				d_unit.SetTempo(std::max(10, d_unit.GetTempo() + amt)); }

			// indicate handled even if not paired with a valid key
			return true; }

			auto it = std::find_if(begin(kGridScanLUT), end(kGridScanLUT),
								   [&](auto &item) { return e.wVirtualScanCode == item; });
			if (it != end(kGridScanLUT)) {
				const int idx = std::distance(begin(kGridScanLUT), it);
				if (reactor.GetKeyState(ScanCode::P)) {
					d_unit.SwitchPattern(idx); }
				else if (d_isRecording) {
					d_unit.ToggleTrackGridNote(d_selectedTrack, idx); }
				else {
					d_unit.Trigger(idx); }
				return true; }}
	return false; }


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
