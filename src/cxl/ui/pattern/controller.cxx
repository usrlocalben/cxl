#include "src/cxl/ui/pattern/controller.hxx"

#include <algorithm>
#include <array>
#include <memory>
#include <string>

#include "src/cxl/log.hxx"
#include "src/cxl/ui/alert/view.hxx"
#include "src/cxl/ui/pattern/page.hxx"
#include "src/cxl/ui/pattern_length_edit/view.hxx"
#include "src/cxl/ui/tap_tempo/view.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_reactor_timer.hxx"
#include "src/rcl/rcls/rcls_console.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace {

const int kAlertDurationInMillis = 500;

constexpr int kKeyRateInHz = 20;

constexpr double kNudgePct = 0.10;

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

constexpr auto kTempoKey{SC::Equals};
constexpr auto kSwingKey{SC::Minus};
constexpr auto kFnKey{SC::LeftCtrl};
constexpr auto kLeftKey{SC::OpenBracket};
constexpr auto kRightKey{SC::CloseBracket};
constexpr auto kDecKnob{SC::Comma};
constexpr auto kIncKnob{SC::Period};
constexpr auto kPatternLengthKey{SC::Backslash};
constexpr auto kRecordKey{SC::L};
constexpr auto kPlayKey{SC::Semicolon};
constexpr auto kStopKey{SC::Quote};
constexpr auto kParamPageKey{SC::Backspace};
constexpr auto kSaveKitKey{SC::F5};
constexpr auto kLoadKitKey{SC::F6};
constexpr auto kDecKitKey{SC::F7};
constexpr auto kIncKitKey{SC::F8};
constexpr auto kPatternLoadKey{SC::P};
constexpr auto kMuteKey{SC::M};


}  // namespace

namespace cxl {

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
	auto result = HandleKeyEvent2(e);
	d_prevKey = e;
	return result; }


void PatternController::KeyboardTick() {
	const auto offset = d_state.knobDir;
	if (offset != 0) {
		if (d_state.curParam) {
			AdjustPageParameter(d_unit, d_state.curVoicePage, d_state.curTrack, d_state.curParam.value(), offset); }
		else if (d_loop.IsKeyDown(kTempoKey)) {
			d_unit.SetTempo(std::max(10, d_unit.GetTempo() + offset)); }
		else if (d_loop.IsKeyDown(kSwingKey)) {
			d_unit.SetSwing(std::clamp(d_unit.GetSwing() + offset, 50, 75)); }
		else {
			return; }
		d_loop.DrawScreenEventually(); }}


bool PatternController::HandleKeyEvent2(const TextKit::KeyEvent e) {
	if (d_patternLengthEditController) {
		auto handled = d_patternLengthEditController->HandleKeyEvent(e);
		if (handled) {
			return true; }}

	const auto key = e.scanCode;
	const auto down = e.down;
	const auto up = !e.down;
	using SC = rcls::ScanCode;

	if (up && key == kFnKey) {
		if (d_state.subMode == SM_TAP_TEMPO) {
			d_view.d_popup.reset();
			d_state.subMode = SM_NONE; }
		return true; }

	if (up && (key == kLeftKey || key == kRightKey)) {
		const int dir = (key == kLeftKey ? -1 : 1);
		StopNudge(dir);
		return true; }

	auto paramSearch = std::find_if(cbegin(kParamScanLUT), cend(kParamScanLUT),
	                                [&](auto &item) { return key == item; });
	if (paramSearch != cend(kParamScanLUT)) {
		const auto paramIdx = std::distance(cbegin(kParamScanLUT), paramSearch);
		if (down) {
			d_state.curParam = paramIdx; }
		else {
			if (d_state.curParam.value_or(-1) == paramIdx) {
				d_state.curParam.reset(); }}
		return true; }


	if (key == kDecKnob || key == kIncKnob) {
		const int dir = key == kDecKnob ? -1 : 1;
		if (down) {
			d_state.knobDir = dir;
			StartTick(); }
		else {
			if (d_state.knobDir == dir) {
				d_state.knobDir = 0;
				StopTick(); }}
		return true; }


	auto gridSearch = std::find_if(cbegin(kGridScanLUT), cend(kGridScanLUT),
	                               [&](auto &item) { return key == item; });
	const int gridIdx = gridSearch != cend(kGridScanLUT) ? std::distance(cbegin(kGridScanLUT), gridSearch) : -1;
	// todo: set flag for parameter locks

	bool fn = (e.control == rcls::kCKLeftCtrl);

	if (!down) {
		return false; }

	if (key == kLeftKey || key == kRightKey) {
		const int dir = (key == kLeftKey ? -1 : 1);
		StartNudge(dir);
		return true; }

	if (key == kPatternLengthKey) {
		// pattern length
		if (fn) {
			// open pattern length edit dialog
			d_patternLengthEditController.emplace(d_unit.GetPatternLength());
			d_view.d_popup = d_patternLengthEditController->d_view;
			d_patternLengthEditController->onSuccess = [&](int newValue) {
				d_view.d_popup.reset();
				d_unit.SetPatternLength(newValue);
				d_patternLengthEditController.reset(); };
			d_patternLengthEditController->onCancel = [&]() {
				d_view.d_popup.reset();
				d_patternLengthEditController.reset(); }; }
		else {
			// select next page
			d_state.curGridPage++;
			if (d_state.curGridPage >= d_unit.GetPatternLength()/16) {
				d_state.curGridPage = 0; }}
		return true; }

	if (key == kSwingKey) {
		if (fn) {
			// tap tempo
			if (d_state.subMode == SM_NONE) {
				if (!d_view.d_popup) {
					d_state.subMode = SM_TAP_TEMPO;
					d_tapTempo.Reset();
					d_taps = 1;
					d_view.d_popup = MakeTapTempoView(d_taps);
					d_tapTempo.Tap(); }}
			else if (d_state.subMode == SM_TAP_TEMPO) {
				d_tapTempo.Tap();
				d_taps++;
				if (d_tapTempo.GetNumTaps() >= 4) {
					Log::GetInstance().info(fmt::sprintf("tapTempo: %.4f", d_tapTempo.GetTempo()));
					auto newTempo = static_cast<int>(d_tapTempo.GetTempo() * 10);
					d_unit.SetTempo(newTempo); }}}
		return true; }

	if (key == kRecordKey) {
		// rec, copy
		if (!fn) {
			// toggle record
			d_state.isRecording = !d_state.isRecording;
			d_unit.CommitPattern(); }
		else if (fn) {
			// copy page if recording
			if (d_state.isRecording) {
				CopyTrackPage();  // copy page
				d_view.d_popup = MakeAlert("copy page");
				rclmt::Delay(kAlertDurationInMillis, [&]() {
					d_view.d_popup.reset();
					d_loop.DrawScreenEventually(); }); }}
		return true; }

	if (key == kPlayKey) {
		// play, clear
		if (!fn) {
			d_unit.Play(); }
		else {
			if (d_state.isRecording) {
				ClearTrackPage();  // clear page
				d_view.d_popup = MakeAlert("clear page");
				rclmt::Delay(kAlertDurationInMillis, [&]() {
					d_view.d_popup.reset();
					d_loop.DrawScreenEventually(); }); }}
		return true; }

	if (key == kStopKey) {
		// stop, paste
		if (!fn) {
			d_unit.Stop(); }
		else {
			if (d_state.isRecording) {
				PasteTrackPage();  // paste page
				d_view.d_popup = MakeAlert("paste page");
				rclmt::Delay(kAlertDurationInMillis, [&]() {
					d_view.d_popup.reset();
					d_loop.DrawScreenEventually(); }); }}
		return true; }

	if (key == kParamPageKey) {
		d_state.curVoicePage = (d_state.curVoicePage+1)%3;
		return true; }

	// kit load/save/change
	if (key == kSaveKitKey) {
		d_unit.SaveKit();
		return true; }
	if (key == kLoadKitKey) {
		d_unit.LoadKit();
		return true; }
	if (key == kDecKitKey) {
		d_unit.DecrementKit();
		return true; }
	if (key == kIncKitKey) {
		d_unit.IncrementKit();
		return true; }

	// grid/track matrix keys 1-16
	if (gridIdx > -1) {
		if (fn) {
			d_state.curTrack = gridIdx; }
		else if (d_loop.IsKeyDown(kPatternLoadKey)) {
			d_unit.SwitchPattern(gridIdx); }
		else if (d_loop.IsKeyDown(kMuteKey)) {
			d_unit.ToggleTrackMute(gridIdx); }
		else if (d_state.isRecording) {
			d_unit.ToggleTrackGridNote(d_state.curTrack, d_state.curGridPage*16+gridIdx); }
		else {
			d_unit.Trigger(gridIdx); }
		return true; }

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


void PatternController::StartTick() {
	if (d_timerId == -1) {
		d_timerId = rclmt::Repeat(1000.0/kKeyRateInHz, [&]() { KeyboardTick(); }); }}


void PatternController::StopTick() {
	if (d_timerId != -1) {
		rclmt::CancelRepeat(d_timerId);
		d_timerId = -1; }}


void PatternController::StartNudge(const int dir) {
	assert(dir == -1 || dir == 1);
	if (d_nudgeDir == dir) {
		/* should be impossible. do nothing */ }
	else {
		if (d_nudgeDir == 0) {
			d_nudgeOldTempo = d_unit.GetTempo(); }
		auto offset = d_nudgeOldTempo * kNudgePct * dir;
		auto newTempo = static_cast<int>(d_nudgeOldTempo + offset);
		d_nudgeDir = dir;
		d_unit.SetTempo(newTempo); }}


void PatternController::StopNudge(const int dir) {
	assert(dir == -1 || dir == 1);
	if (d_nudgeDir == dir) {
		d_nudgeDir = 0;
		d_unit.SetTempo(d_nudgeOldTempo); }}


PatternController::~PatternController() {
	StopTick(); }


}  // namespace cxl
}  // namespace rqdq
