#include "src/cxl/ui/cxl_ui_pattern_editor.hxx"
#include "src/cxl/cxl_reactor.hxx"
#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/ui/cxl_ui_pattern_length_edit.hxx"
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

using SC = rclw::ScanCode;

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


PatternEditor::PatternEditor(CXLUnit& unit)
	:d_unit(unit) {
	auto& reactor = Reactor::GetInstance();

	// XXX dtor should stop listening to these!
	d_unit.d_playbackPositionChanged.connect(this, &PatternEditor::onCXLUnitPlaybackPositionChangedMT);
	reactor.ListenForever({ d_playbackPositionChangedEvent.GetHandle(),
	                        [&]() { onCXLUnitPlaybackPositionChanged(); }});}

void PatternEditor::onCXLUnitPlaybackPositionChangedMT(int pos) {
	// this is called from the ASIO thread.
	// use a Reactor event to bounce to main
	d_playbackPositionChangedEvent.Set(); }
void PatternEditor::onCXLUnitPlaybackPositionChanged() {
	Reactor::GetInstance().DrawScreenEventually(); }


std::pair<int, int> PatternEditor::Pack(int w, int h) {
	return {80, 23}; }


int PatternEditor::GetType() {
	return WT_BOX; }


const rclw::ConsoleCanvas& PatternEditor::Draw(int width, int height) {
	auto& out = d_canvas;
	out.Resize(width, height);
	out.Clear();
	WriteXY(out, 1, 1, DrawTrackSelection());

	WriteXY(out, 1, height-3, DrawGrid());
	WriteXY(out, 68, height-2, DrawPageIndicator());

	WriteXY(out, 8, 5, DrawParameters());

	if (d_popup) {
		auto attr = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlack);
		Fill(out, attr);
		auto [sx, sy] = d_popup->Pack(-1, -1);
		const auto& overlay = d_popup->Draw(sx, sy);
		int xc = (width - overlay.d_width) / 2;
		int yc = (height - overlay.d_height) / 2;
		WriteXY(out, xc, yc, overlay); }

	return out; }


const rclw::ConsoleCanvas& PatternEditor::DrawTrackSelection() {
	static rclw::ConsoleCanvas out{ 25, 2 };
	out.Clear();
	auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlack);
	auto hi = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlue);
	for (int ti = 0; ti < 8; ti++) {
		auto isMuted = d_unit.IsTrackMuted(ti);
		WriteXY(out, ti*3+1, 0, tolower(kTrackNames[ti]), isMuted?lo:hi); }
	for (int ti = 8; ti <16; ti++) {
		auto isMuted = d_unit.IsTrackMuted(ti);
		WriteXY(out, (ti-8)*3+1, 1, tolower(kTrackNames[ti]), isMuted?lo:hi); }

	int selY = d_selectedTrack / 8;
	int selX = (d_selectedTrack % 8) * 3 + 1;
	auto isMuted = d_unit.IsTrackMuted(d_selectedTrack);
	WriteXY(out, selX-1, selY, "[" + kTrackNames[d_selectedTrack] + "]", isMuted?lo:hi);
	return out; }


const std::string PatternEditor::GetPageParameterName(int pageNum, int trackNum, int paramNum) {
	if (pageNum == 0) {
		return d_unit.GetVoiceParameterName(trackNum, paramNum); }
	else if (pageNum == 1) {
		return d_unit.GetEffectParameterName(trackNum, paramNum); }
	else if (pageNum == 2) {
		return d_unit.GetMixParameterName(trackNum, paramNum); }
	else {
		auto msg = fmt::sprintf("invalid parameter page %d", pageNum);
		throw std::runtime_error(msg); }}


int PatternEditor::GetPageParameterValue(int pageNum, int trackNum, int paramNum) {
	if (pageNum == 0) {
		return d_unit.GetVoiceParameterValue(trackNum, paramNum); }
	else if (pageNum == 1) {
		return d_unit.GetEffectParameterValue(trackNum, paramNum); }
	else if (pageNum == 2) {
		return d_unit.GetMixParameterValue(trackNum, paramNum); }
	else {
		auto msg = fmt::sprintf("invalid parameter page %d", pageNum);
		throw std::runtime_error(msg); }}


void PatternEditor::AdjustPageParameter(int pageNum, int trackNum, int paramNum, int offset) {
	if (pageNum == 0) {
		d_unit.AdjustVoiceParameter(trackNum, paramNum, offset); }
	else if (pageNum == 1) {
		d_unit.AdjustEffectParameter(trackNum, paramNum, offset); }
	else if (pageNum == 2) {
		d_unit.AdjustMixParameter(trackNum, paramNum, offset); }
	else {
		auto msg = fmt::sprintf("invalid parameter page %d", pageNum);
		throw std::runtime_error(msg); }}


const rclw::ConsoleCanvas& PatternEditor::DrawParameters() {
	static rclw::ConsoleCanvas out{ 7*4, 1+2*2+1 };
	out.Clear();
	Fill(out, rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Cyan));

	auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Brown);
	auto hi = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBrown);

	WriteXY(out, 0, 0, "Voice", d_selectedVoicePage==0?hi:lo);
	WriteXY(out, 7, 0, "Effect", d_selectedVoicePage==1?hi:lo);
	WriteXY(out, 15, 0, "Mix", d_selectedVoicePage==2?hi:lo);

	// row 1
	int page = d_selectedVoicePage;

	int row = 0;
	for (int pi=0; pi<4; pi++) {
		const auto paramName = GetPageParameterName(page, d_selectedTrack, pi);
		if (!paramName.empty()) {
			int value = GetPageParameterValue(page, d_selectedTrack, pi);
			WriteXY(out, 7*pi+2, row*2+0+1, paramName);
			WriteXY(out, 7*pi+0, row*2+1+1, fmt::sprintf("% 3d", value));}}

	// row 2
	row = 1;
	for (int pi=4; pi<8; pi++) {
		const auto paramName = GetPageParameterName(page, d_selectedTrack, pi);
		if (!paramName.empty()) {
			int value = GetPageParameterValue(page, d_selectedTrack, pi);
			WriteXY(out, 7*(pi-4)+2, row*2+0+1, paramName);
			WriteXY(out, 7*(pi-4)+0, row*2+1+1, fmt::sprintf("% 3d", value));}}

	int waveId = d_unit.GetVoiceParameterValue(d_selectedTrack, 0);
	std::string waveName = d_unit.GetWaveName(waveId);
	WriteXY(out, 0, 4+1, fmt::sprintf("Wave: %s", waveName));
	return out; }


const rclw::ConsoleCanvas& PatternEditor::DrawGrid() {
	static rclw::ConsoleCanvas out{ 65, 3 };
	auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Brown);
	auto hi = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBrown);
	auto red = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongRed);
	auto dark = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBlack);
	Fill(out, lo);
	WriteXY(out, 0, 0, "| .   .   .   . | .   .   .   . | .   .   .   . | .   .   .   . | ");
	const int curPos = d_unit.GetLastPlayedGridPosition();
	const int curPage = curPos/16;
	const int curPagePos = curPos%16;

	if (curPage == d_selectedGridPage) {
		WriteXY(out, 2+curPagePos*4, 0, "o", hi); }

	WriteXY(out, 0, 1, "|");
	for (int i = 0; i < 16; i++) {
		auto value = d_unit.GetTrackGridNote(d_selectedTrack, d_selectedGridPage*16+i);
		WriteXY(out, i*4+2, 1, value != 0 ? "X" : " ", red);
		WriteXY(out, i*4+4, 1, "|", lo); }

	if (d_isRecording) {
		WriteXY(out, 0, 2, "REC", red); }
	else {
		WriteXY(out, 0, 2, "rec", dark); }
	return out; }


const rclw::ConsoleCanvas& PatternEditor::DrawPageIndicator() {
	static rclw::ConsoleCanvas out{ 9, 1 };
	int curPage = d_selectedGridPage;
	int playingPage = d_unit.GetLastPlayedGridPosition() / 16;

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


bool PatternEditor::HandleKeyEvent(const KEY_EVENT_RECORD e) {
	if (d_popup) {
		bool handled = d_popup->HandleKeyEvent(e);
		if (handled) {
			return true; }}

	auto& reactor = Reactor::GetInstance();
	if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && (ScanCode::Key1<=e.wVirtualScanCode && e.wVirtualScanCode<=ScanCode::Key8)) {
		// Ctrl+1...Ctrl+8
		d_selectedTrack = e.wVirtualScanCode - ScanCode::Key1;
		return true; }
	if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && e.wVirtualScanCode==ScanCode::Backslash) {
		auto editor = std::make_shared<PatternLengthEdit>(d_unit.GetPatternLength());
		d_popup = std::make_shared<LineBox>(editor);
		editor->onSuccess = [&](int newValue) {
			d_popup.reset();
			d_unit.SetPatternLength(newValue); };
		editor->onCancel = [&]() {
			d_popup.reset(); };
		return true; }
	if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && e.wVirtualScanCode==ScanCode::L && d_isRecording) {
		CopyTrackPage();  // copy page 
		return true; }
	if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && e.wVirtualScanCode==ScanCode::Semicolon && d_isRecording) {
		ClearTrackPage();  // clear page 
		return true; }
	if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && e.wVirtualScanCode==ScanCode::Quote && d_isRecording) {
		PasteTrackPage();  // paste page 
		return true; }

	// xxx if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCK
	if ((e.bKeyDown != 0) && e.dwControlKeyState==0) {
		if (e.wVirtualScanCode == ScanCode::Backslash) {
			d_selectedGridPage++;
			if (d_selectedGridPage >= d_unit.GetPatternLength()/16) {
				d_selectedGridPage = 0; }
			return true; }

		// parameter edit page
		if (e.wVirtualScanCode == ScanCode::Backspace) {
			d_selectedVoicePage = (d_selectedVoicePage+1)%3;
			return true; }

		// transport controls, copy/clear/paste
		if (e.wVirtualScanCode == ScanCode::L) {
			d_isRecording = !d_isRecording;
			d_unit.CommitPattern();
			return true; }
		if (e.wVirtualScanCode == ScanCode::Semicolon) {
			d_unit.Play();
			return true; }
		if (e.wVirtualScanCode == ScanCode::Quote) {
			d_unit.Stop();
			return true; }

		// kit load/save/change
		if (e.wVirtualScanCode == ScanCode::F5) {
			d_unit.SaveKit();
			return true; }
		if (e.wVirtualScanCode == ScanCode::F6) {
			d_unit.LoadKit();
			return true; }
		if (e.wVirtualScanCode == ScanCode::F7) {
			d_unit.DecrementKit();
			return true; }
		if (e.wVirtualScanCode == ScanCode::F8) {
			d_unit.IncrementKit();
			return true; }

		// value inc/dec
		if (e.wVirtualScanCode == ScanCode::Comma || e.wVirtualScanCode == ScanCode::Period) {

			int offset = (e.wVirtualScanCode == ScanCode::Comma ? -1 : 1);
			if ((e.dwControlKeyState & rclw::kCKShift) != 0u) {
				// XXX does not work because of MSFT internal bug 9311951
				// https://github.com/Microsoft/WSL/issues/1188
				offset *= 10; }

			auto it = std::find_if(begin(kParamScanLUT), end(kParamScanLUT),
								   [&](auto& item) { return reactor.GetKeyState(item); });
			if (it != end(kParamScanLUT)) {
				int idx = std::distance(begin(kParamScanLUT), it);
				AdjustPageParameter(d_selectedVoicePage, d_selectedTrack, idx, offset); }
			else if (reactor.GetKeyState(ScanCode::Equals)) {
				d_unit.SetTempo(std::max(10, d_unit.GetTempo() + offset)); }

			// indicate handled even if not paired with a valid key
			return true; }

		// grid/track matrix keys 1-16
		auto it = std::find_if(begin(kGridScanLUT), end(kGridScanLUT),
							   [&](auto &item) { return e.wVirtualScanCode == item; });
		if (it != end(kGridScanLUT)) {
			const int idx = std::distance(begin(kGridScanLUT), it);
			if (reactor.GetKeyState(ScanCode::P)) {
				d_unit.SwitchPattern(idx); }
			else if (reactor.GetKeyState(ScanCode::M)) {
				d_unit.ToggleTrackMute(idx); }
			else if (d_isRecording) {
				d_unit.ToggleTrackGridNote(d_selectedTrack, d_selectedGridPage*16+idx); }
			else {
				d_unit.Trigger(idx); }
			return true; }}

	return false; }


void PatternEditor::CopyTrackPage() {
	for (int i=0; i<16; i++) {
		auto note = d_unit.GetTrackGridNote(d_selectedTrack, d_selectedGridPage*16+i);
		d_clipboard[i] = note; }}


void PatternEditor::ClearTrackPage() {
	for (int i=0; i<16; i++) {
		d_unit.SetTrackGridNote(d_selectedTrack, d_selectedGridPage*16+i, 0); }}


void PatternEditor::PasteTrackPage() {
	for (int i=0; i<16; i++) {
		d_unit.SetTrackGridNote(d_selectedTrack, d_selectedGridPage*16+i, d_clipboard[i]); }}



}  // namespace cxl
}  // namespace rqdq
