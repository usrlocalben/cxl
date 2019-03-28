#include "sequencer.hxx"

#include <algorithm>
#include <cmath>
#include <optional>

#include "src/ral/raldsp/raldsp_sampler.hxx"

namespace rqdq {
namespace {

constexpr int kEighthNoteInPPQ{48};

constexpr int kPPQ{96};

/**
 * compute the duration (in ppq) of a given noteIdx given
 * a swing% value (integer, [50, 75])
 */
int ComputeSwing(int noteIdx, int swingPct) {
	auto firstHalf = lround(kEighthNoteInPPQ * swingPct / 100.0);
	auto remainder = kEighthNoteInPPQ - firstHalf;
	return noteIdx % 2 == 0 ? firstHalf : remainder; }


/**
 * compute the duration of one ppq point (in samples) given
 * a tempo (in BPM)
 */
double ComputeSamplesPerPoint(double bpm) {
	auto oneBeatInSamples = 44100.0 / (bpm/60.0);
	auto onePointInSamples = oneBeatInSamples / kPPQ;
	return onePointInSamples; }


}  // namespace

namespace cxl {

GridSequencer::GridSequencer() = default;


void GridSequencer::SetTempo(const int bpm) {
	tempoInBPM_ = bpm; }


int GridSequencer::GetTempo() const {
	return tempoInBPM_; }


void GridSequencer::SetSwing(int pct) {
	swingPct_ = std::clamp(pct, 50, 75); }


int GridSequencer::GetSwing() const {
	return swingPct_; }


void GridSequencer::AddTrack(raldsp::SingleSampler& voice, std::optional<int> muteGroupId) {
	tracks_.emplace_back(&voice, muteGroupId); }


bool GridSequencer::Update() {
	bool updated = false;
	if (state_==PlayerState::Stopped && wantedState_==PlayerState::Playing) {
		updated = true;
		state_ = PlayerState::Playing;
		pointSamplesError_ = 0;
		pointTimeRemainingInSamples_ = 0;
		noteTimeRemainingInPoints_ = 0;
		noteIdx_ = -1;
		timeInPoints_ = 0; }
	if (state_==PlayerState::Playing && wantedState_==PlayerState::Stopped) {
		updated = true;
		state_ = PlayerState::Stopped;
		timeInPoints_ = 0; }
	return updated; }


bool GridSequencer::Process() {
	bool updated = false;
	if (state_ == PlayerState::Playing) {
		if (--pointTimeRemainingInSamples_ <= 0) {
			auto onePointInSamples = ComputeSamplesPerPoint(tempoInBPM_/10.0);
			auto needed = onePointInSamples + pointSamplesError_;
			pointTimeRemainingInSamples_ = static_cast<int>(needed);
			pointSamplesError_ = needed - pointTimeRemainingInSamples_;
			timeInPoints_++;
			if (--noteTimeRemainingInPoints_ <= 0) {
				noteIdx_ = (noteIdx_ + 1) % patternLength_;
				noteTimeRemainingInPoints_ = ComputeSwing(noteIdx_, swingPct_);
				updated = true;
				TriggerCurrentNote(); }}}
	return updated; }


void GridSequencer::TriggerCurrentNote() {
	for (auto& track : tracks_) {
		if (track.grid[noteIdx_] != 0 && !track.isMuted) {
			track.voice->Trigger(48, 1.0, 0);
			if (track.muteGroupId) {
				for (auto& other : tracks_) {
					if (&track != &other &&
						other.muteGroupId &&
						other.muteGroupId.value() == track.muteGroupId.value()) {
						other.voice->Stop(); }}}}}}


}  // namespace cxl
}  // namespace rqdq
