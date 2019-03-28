#include "controller.hxx"

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
constexpr std::array kGridScanLUT = {
	SC::Key1, SC::Key2, SC::Key3, SC::Key4,
	SC::Q,    SC::W,    SC::E,    SC::R,
	SC::A,    SC::S,    SC::D,    SC::F,
	SC::Z,    SC::X,    SC::C,    SC::V };

/**
 * LUT for converting a 4x4 matrix of scan-codes to
 * indices for parameter controls
 */
constexpr std::array kParamScanLUT = {
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
	:unit_(unit), loop_(loop), view_(unit_, state_) {
	auto& reactor = rclmt::Reactor::GetInstance();

	// XXX dtor should stop listening to these!
	signalId_ = unit_.ConnectPlaybackPositionChanged([&](int pos) {
		// this is called from the ASIO thread.
		// use a Reactor event to bounce to main
		playbackPositionChangedEvent_.Signal(); });
	reactor.ListenMany(playbackPositionChangedEvent_, [&]() {
		loop_.DrawScreenEventually(); }); }


bool PatternController::HandleKeyEvent(const TextKit::KeyEvent e) {
	auto result = HandleKeyEvent2(e);
	prevKey_ = e;
	return result; }


void PatternController::KeyboardTick() {
	const auto offset = state_.knobDir;
	if (offset != 0) {
		if (state_.curParam) {
			AdjustPageParameter(unit_, state_.curVoicePage, state_.curTrack, state_.curParam.value(), offset); }
		else if (loop_.IsKeyDown(kTempoKey)) {
			unit_.SetTempo(std::max(10, unit_.GetTempo() + offset)); }
		else if (loop_.IsKeyDown(kSwingKey)) {
			unit_.SetSwing(std::clamp(unit_.GetSwing() + offset, 50, 75)); }
		else {
			return; }
		loop_.DrawScreenEventually(); }}


bool PatternController::HandleKeyEvent2(const TextKit::KeyEvent e) {
	if (patternLengthEditController_) {
		auto handled = patternLengthEditController_->HandleKeyEvent(e);
		if (handled) {
			return true; }}

	const auto key = e.scanCode;
	const auto down = e.down;
	const auto up = !e.down;
	using SC = rcls::ScanCode;

	if (up && key == kFnKey) {
		if (state_.subMode == SM_TAP_TEMPO) {
			view_.popup_.reset();
			state_.subMode = SM_NONE; }
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
			state_.curParam = paramIdx; }
		else {
			if (state_.curParam.value_or(-1) == paramIdx) {
				state_.curParam.reset(); }}
		return true; }


	if (key == kDecKnob || key == kIncKnob) {
		const int dir = key == kDecKnob ? -1 : 1;
		if (down) {
			state_.knobDir = dir;
			StartTick(); }
		else {
			if (state_.knobDir == dir) {
				state_.knobDir = 0;
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
			patternLengthEditController_.emplace(unit_.GetPatternLength());
			view_.popup_ = patternLengthEditController_->view_;
			patternLengthEditController_->onSuccess = [&](int newValue) {
				view_.popup_.reset();
				unit_.SetPatternLength(newValue);
				patternLengthEditController_.reset(); };
			patternLengthEditController_->onCancel = [&]() {
				view_.popup_.reset();
				patternLengthEditController_.reset(); }; }
		else {
			// select next page
			state_.curGridPage++;
			if (state_.curGridPage >= unit_.GetPatternLength()/16) {
				state_.curGridPage = 0; }}
		return true; }

	if (key == kTempoKey) {
		if (fn) {
			// tap tempo
			if (state_.subMode == SM_NONE) {
				if (!view_.popup_) {
					state_.subMode = SM_TAP_TEMPO;
					tapTempo_.Reset();
					taps_ = 1;
					view_.popup_ = MakeTapTempoView(taps_);
					tapTempo_.Tap(); }}
			else if (state_.subMode == SM_TAP_TEMPO) {
				tapTempo_.Tap();
				taps_++;
				if (tapTempo_.GetNumTaps() >= 4) {
					Log::GetInstance().info(fmt::sprintf("tapTempo: %.4f", tapTempo_.GetTempo()));
					auto newTempo = static_cast<int>(tapTempo_.GetTempo() * 10);
					unit_.SetTempo(newTempo); }}}
		return true; }

	if (key == kRecordKey) {
		// rec, copy
		if (!fn) {
			// toggle record
			state_.isRecording = !state_.isRecording;
			unit_.CommitPattern(); }
		else if (fn) {
			// copy page if recording
			if (state_.isRecording) {
				CopyTrackPage();  // copy page
				view_.popup_ = MakeAlert("copy page");
				rclmt::Delay(kAlertDurationInMillis, [&]() {
					view_.popup_.reset();
					loop_.DrawScreenEventually(); }); }}
		return true; }

	if (key == kPlayKey) {
		// play, clear
		if (!fn) {
			unit_.Play(); }
		else {
			if (state_.isRecording) {
				ClearTrackPage();  // clear page
				view_.popup_ = MakeAlert("clear page");
				rclmt::Delay(kAlertDurationInMillis, [&]() {
					view_.popup_.reset();
					loop_.DrawScreenEventually(); }); }}
		return true; }

	if (key == kStopKey) {
		// stop, paste
		if (!fn) {
			unit_.Stop(); }
		else {
			if (state_.isRecording) {
				PasteTrackPage();  // paste page
				view_.popup_ = MakeAlert("paste page");
				rclmt::Delay(kAlertDurationInMillis, [&]() {
					view_.popup_.reset();
					loop_.DrawScreenEventually(); }); }}
		return true; }

	if (key == kParamPageKey) {
		state_.curVoicePage = (state_.curVoicePage+1)%3;
		return true; }

	// kit load/save/change
	if (key == kSaveKitKey) {
		unit_.SaveKit();
		return true; }
	if (key == kLoadKitKey) {
		unit_.LoadKit();
		return true; }
	if (key == kDecKitKey) {
		unit_.DecrementKit();
		return true; }
	if (key == kIncKitKey) {
		unit_.IncrementKit();
		return true; }

	// grid/track matrix keys 1-16
	if (gridIdx > -1) {
		if (fn) {
			state_.curTrack = gridIdx; }
		else if (loop_.IsKeyDown(kPatternLoadKey)) {
			unit_.SwitchPattern(gridIdx); }
		else if (loop_.IsKeyDown(kMuteKey)) {
			unit_.ToggleTrackMute(gridIdx); }
		else if (state_.isRecording) {
			unit_.ToggleTrackGridNote(state_.curTrack, state_.curGridPage*16+gridIdx); }
		else {
			unit_.Trigger(gridIdx); }
		return true; }

	return false; }


void PatternController::CopyTrackPage() {
	for (int i=0; i<16; i++) {
		auto note = unit_.GetTrackGridNote(state_.curTrack, state_.curGridPage*16+i);
		clipboard_[i] = note; }}


void PatternController::ClearTrackPage() {
	for (int i=0; i<16; i++) {
		unit_.SetTrackGridNote(state_.curTrack, state_.curGridPage*16+i, 0); }}


void PatternController::PasteTrackPage() {
	for (int i=0; i<16; i++) {
		unit_.SetTrackGridNote(state_.curTrack, state_.curGridPage*16+i, clipboard_[i]); }}


void PatternController::StartTick() {
	if (timerId_ == -1) {
		timerId_ = rclmt::Repeat(1000.0/kKeyRateInHz, [&]() { KeyboardTick(); }); }}


void PatternController::StopTick() {
	if (timerId_ != -1) {
		rclmt::CancelRepeat(timerId_);
		timerId_ = -1; }}


void PatternController::StartNudge(const int dir) {
	assert(dir == -1 || dir == 1);
	if (nudgeDir_ == dir) {
		/* should be impossible. do nothing */ }
	else {
		if (nudgeDir_ == 0) {
			nudgeOldTempo_ = unit_.GetTempo(); }
		auto offset = nudgeOldTempo_ * kNudgePct * dir;
		auto newTempo = static_cast<int>(nudgeOldTempo_ + offset);
		nudgeDir_ = dir;
		unit_.SetTempo(newTempo); }}


void PatternController::StopNudge(const int dir) {
	assert(dir == -1 || dir == 1);
	if (nudgeDir_ == dir) {
		nudgeDir_ = 0;
		unit_.SetTempo(nudgeOldTempo_); }}


PatternController::~PatternController() {
	unit_.DisconnectPlaybackPositionChanged(signalId_);
	StopTick(); }


}  // namespace cxl
}  // namespace rqdq
