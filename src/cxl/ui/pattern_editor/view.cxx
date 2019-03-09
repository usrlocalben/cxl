#include "src/cxl/ui/pattern_editor/view.hxx"

#include <array>
#include <deque>
#include <sstream>
#include <string>
#include <vector>

#include "src/cxl/log.hxx"
#include "src/cxl/ui/alert/view.hxx"
#include "src/cxl/ui/pattern_length_edit/view.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_reactor_delay.hxx"
#include "src/rcl/rcls/rcls_console.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

#include <Windows.h>
#include <fmt/printf.h>

namespace rqdq {
namespace {

const int kAlertDurationInMillis = 500;

using SC = rcls::ScanCode;

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

using ScanCode = rcls::ScanCode;


PatternEditor::PatternEditor(CXLUnit& unit, TextKit::MainLoop& loop)
	:d_unit(unit), d_loop(loop) {
	auto& reactor = rclmt::Reactor::GetInstance();

	// XXX dtor should stop listening to these!
	d_unit.d_playbackPositionChanged.connect(this, &PatternEditor::onCXLUnitPlaybackPositionChangedASIO);
	reactor.ListenMany(d_playbackPositionChangedEvent,
	                   [&]() { onCXLUnitPlaybackPositionChanged(); });}

void PatternEditor::onCXLUnitPlaybackPositionChangedASIO(int pos) {
	// this is called from the ASIO thread.
	// use a Reactor event to bounce to main
	d_playbackPositionChangedEvent.Signal(); }
void PatternEditor::onCXLUnitPlaybackPositionChanged() {
	d_loop.DrawScreenEventually(); }


std::pair<int, int> PatternEditor::Pack(int w, int h) {
	return {80, 23}; }


int PatternEditor::GetType() {
	return TextKit::WT_BOX; }


const rcls::TextCanvas& PatternEditor::Draw(int width, int height) {
	auto& out = d_canvas;
	out.Resize(width, height);
	out.Clear();
	WriteXY(out, 1, 1, DrawTrackSelection());

	WriteXY(out, 1, height-3, DrawGrid());
	WriteXY(out, 68, height-2, DrawPageIndicator());

	WriteXY(out, 8, 5, DrawParameters());

	if (d_popup) {
		auto attr = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack);
		Fill(out, attr);
		auto [sx, sy] = d_popup->Pack(-1, -1);
		const auto& overlay = d_popup->Draw(sx, sy);
		int xc = (width - overlay.d_width) / 2;
		int yc = (height - overlay.d_height) / 2;
		WriteXY(out, xc, yc, overlay); }

	return out; }


const rcls::TextCanvas& PatternEditor::DrawTrackSelection() {
	static rcls::TextCanvas out{ 25, 2 };
	out.Clear();
	auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack);
	auto hi = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlue);
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
	if (pageNum == 1) {
		return d_unit.GetEffectParameterName(trackNum, paramNum); }
	if (pageNum == 2) {
		return d_unit.GetMixParameterName(trackNum, paramNum); }
	auto msg = fmt::sprintf("invalid parameter page %d", pageNum);
	throw std::runtime_error(msg); }


int PatternEditor::GetPageParameterValue(int pageNum, int trackNum, int paramNum) {
	if (pageNum == 0) {
		return d_unit.GetVoiceParameterValue(trackNum, paramNum); }
	if (pageNum == 1) {
		return d_unit.GetEffectParameterValue(trackNum, paramNum); }
	if (pageNum == 2) {
		return d_unit.GetMixParameterValue(trackNum, paramNum); }
	auto msg = fmt::sprintf("invalid parameter page %d", pageNum);
	throw std::runtime_error(msg); }


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


const rcls::TextCanvas& PatternEditor::DrawParameters() {
	static rcls::TextCanvas out{ 7*4, 1+2*2+1 };
	out.Clear();
	Fill(out, rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Cyan));

	auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Brown);
	auto hi = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBrown);

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


const rcls::TextCanvas& PatternEditor::DrawGrid() {
	static rcls::TextCanvas out{ 65, 3 };
	auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Brown);
	auto hi = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBrown);
	auto red = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongRed);
	auto dark = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack);
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


const rcls::TextCanvas& PatternEditor::DrawPageIndicator() {
	static rcls::TextCanvas out{ 9, 1 };
	int curPage = d_selectedGridPage;
	int playingPage = d_unit.GetLastPlayedGridPosition() / 16;

	auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Brown);
	auto hi = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBrown);
	auto red = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongRed);
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


bool PatternEditor::HandleKeyEvent(const TextKit::KeyEvent e) {
	auto& reactor = rclmt::Reactor::GetInstance();
	if (d_popup) {
		bool handled = d_popup->HandleKeyEvent(e);
		if (handled) {
			return true; }}

	if (e.down && e.control==rcls::kCKLeftCtrl && (ScanCode::Key1<=e.scanCode && e.scanCode<=ScanCode::Key8)) {
		// Ctrl+1...Ctrl+8
		d_selectedTrack = e.scanCode - ScanCode::Key1;
		return true; }
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::Backslash) {
		auto editor = std::make_shared<PatternLengthEdit>(d_unit.GetPatternLength());
		d_popup = std::make_shared<TextKit::LineBox>(editor);
		editor->onSuccess = [&](int newValue) {
			d_popup.reset();
			d_unit.SetPatternLength(newValue); };
		editor->onCancel = [&]() {
			d_popup.reset(); };
		return true; }
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::L && d_isRecording) {
		CopyTrackPage();  // copy page
		d_popup = MakeAlert("copy page");
		rclmt::Delay(kAlertDurationInMillis, [&]() {
			d_popup.reset();
			d_loop.DrawScreenEventually(); });
		return true; }
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::Semicolon && d_isRecording) {
		ClearTrackPage();  // clear page
		d_popup = MakeAlert("clear page");
		rclmt::Delay(kAlertDurationInMillis, [&]() {
			d_popup.reset();
			d_loop.DrawScreenEventually(); });
		return true; }
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::Quote && d_isRecording) {
		PasteTrackPage();  // paste page
		d_popup = MakeAlert("paste page");
		rclmt::Delay(kAlertDurationInMillis, [&]() {
			d_popup.reset();
			d_loop.DrawScreenEventually(); });
		return true; }

	// xxx if (e.down && e.control==rcls::kCK
	if (e.down && e.control==0) {
		if (e.scanCode == ScanCode::Backslash) {
			d_selectedGridPage++;
			if (d_selectedGridPage >= d_unit.GetPatternLength()/16) {
				d_selectedGridPage = 0; }
			return true; }

		// parameter edit page
		if (e.scanCode == ScanCode::Backspace) {
			d_selectedVoicePage = (d_selectedVoicePage+1)%3;
			return true; }

		// transport controls, copy/clear/paste
		if (e.scanCode == ScanCode::L) {
			d_isRecording = !d_isRecording;
			d_unit.CommitPattern();
			return true; }
		if (e.scanCode == ScanCode::Semicolon) {
			d_unit.Play();
			return true; }
		if (e.scanCode == ScanCode::Quote) {
			d_unit.Stop();
			return true; }

		// kit load/save/change
		if (e.scanCode == ScanCode::F5) {
			d_unit.SaveKit();
			return true; }
		if (e.scanCode == ScanCode::F6) {
			d_unit.LoadKit();
			return true; }
		if (e.scanCode == ScanCode::F7) {
			d_unit.DecrementKit();
			return true; }
		if (e.scanCode == ScanCode::F8) {
			d_unit.IncrementKit();
			return true; }

		// value inc/dec
		if (e.scanCode == ScanCode::Comma || e.scanCode == ScanCode::Period) {

			int offset = (e.scanCode == ScanCode::Comma ? -1 : 1);
			if ((e.control & rcls::kCKShift) != 0u) {
				// XXX does not work because of MSFT internal bug 9311951
				// https://github.com/Microsoft/WSL/issues/1188
				offset *= 10; }

			auto it = std::find_if(begin(kParamScanLUT), end(kParamScanLUT),
								   [&](auto& item) { return d_loop.IsKeyDown(item); });
			if (it != end(kParamScanLUT)) {
				int idx = std::distance(begin(kParamScanLUT), it);
				AdjustPageParameter(d_selectedVoicePage, d_selectedTrack, idx, offset); }
			else if (d_loop.IsKeyDown(ScanCode::Equals)) {
				d_unit.SetTempo(std::max(10, d_unit.GetTempo() + offset)); }

			// indicate handled even if not paired with a valid key
			return true; }

		// grid/track matrix keys 1-16
		auto it = std::find_if(begin(kGridScanLUT), end(kGridScanLUT),
							   [&](auto &item) { return e.scanCode == item; });
		if (it != end(kGridScanLUT)) {
			const int idx = std::distance(begin(kGridScanLUT), it);
			if (d_loop.IsKeyDown(ScanCode::P)) {
				d_unit.SwitchPattern(idx); }
			else if (d_loop.IsKeyDown(ScanCode::M)) {
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
