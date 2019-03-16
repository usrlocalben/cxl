#include "src/cxl/ui/pattern/controller.hxx"

#include <algorithm>
#include <array>
#include <memory>
#include <string>

#include "src/cxl/log.hxx"
#include "src/cxl/ui/alert/view.hxx"
#include "src/cxl/ui/pattern/page.hxx"
#include "src/cxl/ui/pattern_length_edit/view.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_reactor_delay.hxx"
#include "src/rcl/rcls/rcls_console.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"

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


PatternController::PatternController(CXLUnit& unit, TextKit::MainLoop& loop)
	:d_unit(unit), d_loop(loop), d_view(d_unit, d_state) {
	auto& reactor = rclmt::Reactor::GetInstance();

	// XXX dtor should stop listening to these!
	d_unit.d_playbackPositionChanged.connect(this, &PatternController::onCXLUnitPlaybackPositionChangedASIO);
	reactor.ListenMany(d_playbackPositionChangedEvent,
	                   [&]() { onCXLUnitPlaybackPositionChanged(); });}

void PatternController::onCXLUnitPlaybackPositionChangedASIO(int pos) {
	// this is called from the ASIO thread.
	// use a Reactor event to bounce to main
	d_playbackPositionChangedEvent.Signal(); }
void PatternController::onCXLUnitPlaybackPositionChanged() {
	d_loop.DrawScreenEventually(); }


bool PatternController::HandleKeyEvent(const TextKit::KeyEvent e) {
	if (d_patternLengthEditController) {
		bool handled = d_patternLengthEditController->HandleKeyEvent(e);
		if (handled) {
			return true; }}

	if (e.down && e.control==rcls::kCKLeftCtrl && (ScanCode::Key1<=e.scanCode && e.scanCode<=ScanCode::Key8)) {
		// Ctrl+1...Ctrl+8
		d_state.curTrack = e.scanCode - ScanCode::Key1;
		return true; }
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::Backslash) {
        d_patternLengthEditController.emplace(d_unit.GetPatternLength());
		d_view.d_popup = std::make_shared<TextKit::LineBox>(&(d_patternLengthEditController->d_view));
		d_patternLengthEditController->onSuccess = [&](int newValue) {
			d_view.d_popup.reset();
			d_unit.SetPatternLength(newValue);
            d_patternLengthEditController.reset(); };
		d_patternLengthEditController->onCancel = [&]() {
			d_view.d_popup.reset();
            d_patternLengthEditController.reset(); };
		return true; }
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::L && d_state.isRecording) {
		CopyTrackPage();  // copy page
		d_view.d_popup = MakeAlert("copy page");
		rclmt::Delay(kAlertDurationInMillis, [&]() {
			d_view.d_popup.reset();
			d_loop.DrawScreenEventually(); });
		return true; }
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::Semicolon && d_state.isRecording) {
		ClearTrackPage();  // clear page
		d_view.d_popup = MakeAlert("clear page");
		rclmt::Delay(kAlertDurationInMillis, [&]() {
			d_view.d_popup.reset();
			d_loop.DrawScreenEventually(); });
		return true; }
	if (e.down && e.control==rcls::kCKLeftCtrl && e.scanCode==ScanCode::Quote && d_state.isRecording) {
		PasteTrackPage();  // paste page
		d_view.d_popup = MakeAlert("paste page");
		rclmt::Delay(kAlertDurationInMillis, [&]() {
			d_view.d_popup.reset();
			d_loop.DrawScreenEventually(); });
		return true; }

	// xxx if (e.down && e.control==rcls::kCK
	if (e.down && e.control==0) {
		if (e.scanCode == ScanCode::Backslash) {
			d_state.curGridPage++;
			if (d_state.curGridPage >= d_unit.GetPatternLength()/16) {
				d_state.curGridPage = 0; }
			return true; }

		// parameter edit page
		if (e.scanCode == ScanCode::Backspace) {
			d_state.curVoicePage = (d_state.curVoicePage+1)%3;
			return true; }

		// transport controls, copy/clear/paste
		if (e.scanCode == ScanCode::L) {
			d_state.isRecording = !d_state.isRecording;
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
				AdjustPageParameter(d_unit, d_state.curVoicePage, d_state.curTrack, idx, offset); }
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
			else if (d_state.isRecording) {
				d_unit.ToggleTrackGridNote(d_state.curTrack, d_state.curGridPage*16+idx); }
			else {
				d_unit.Trigger(idx); }
			return true; }}

	return false; }


void PatternController::CopyTrackPage() {
	for (int i=0; i<16; i++) {
		auto note = d_unit.GetTrackGridNote(d_state.curTrack, d_state.curGridPage*16+i);
		d_clipboard[i] = note; }}


void PatternController::ClearTrackPage() {
	for (int i=0; i<16; i++) {
		d_unit.SetTrackGridNote(d_state.curTrack, d_state.curGridPage*16+i, 0); }}


void PatternController::PasteTrackPage() {
	for (int i=0; i<16; i++) {
		d_unit.SetTrackGridNote(d_state.curTrack, d_state.curGridPage*16+i, d_clipboard[i]); }}


}  // namespace cxl
}  // namespace rqdq
