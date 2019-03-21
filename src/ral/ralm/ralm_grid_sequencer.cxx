#include "src/ral/ralm/ralm_grid_sequencer.hxx"

#include <algorithm>
#include <cmath>
#include <iostream>

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

namespace ralm {

GridSequencer::GridSequencer() = default;


void GridSequencer::SetTempo(const int bpm) {
	d_tempoInBPM = bpm; }


int GridSequencer::GetTempo() const {
	return d_tempoInBPM; }


void GridSequencer::SetSwing(int pct) {
	d_swingPct = std::clamp(pct, 50, 75); }


int GridSequencer::GetSwing() const {
	return d_swingPct; }


void GridSequencer::AddTrack(raldsp::SingleSampler& voice, std::optional<int> muteGroupId) {
	d_tracks.emplace_back(&voice, muteGroupId); }


bool GridSequencer::Update() {
	bool updated = false;
	if (d_state==PlayerState::Stopped && d_nextState==PlayerState::Playing) {
		updated = true;
		d_state = PlayerState::Playing;
		d_pointSamplesError = 0;
		d_pointTimeRemainingInSamples = 0;
		d_noteTimeRemainingInPoints = 0;
		d_noteIdx = -1;
		d_timeInPoints = 0; }
	if (d_state==PlayerState::Playing && d_nextState==PlayerState::Stopped) {
		updated = true;
		d_state = PlayerState::Stopped;
		d_timeInPoints = 0; }
	return updated; }


bool GridSequencer::Process() {
	bool updated = false;
	if (d_state == PlayerState::Playing) {
		if (--d_pointTimeRemainingInSamples <= 0) {
			auto onePointInSamples = ComputeSamplesPerPoint(d_tempoInBPM/10.0);
			auto needed = onePointInSamples + d_pointSamplesError;
			d_pointTimeRemainingInSamples = static_cast<int>(needed);
			d_pointSamplesError = needed - d_pointTimeRemainingInSamples;
			d_timeInPoints++;
			if (--d_noteTimeRemainingInPoints <= 0) {
				d_noteIdx = (d_noteIdx + 1) % d_patternLength;
				d_noteTimeRemainingInPoints = ComputeSwing(d_noteIdx, d_swingPct);
				updated = true;
				TriggerCurrentNote(); }}}
	return updated; }


void GridSequencer::TriggerCurrentNote() {
	for (auto& track : d_tracks) {
		if (track.grid[d_noteIdx] != 0 && !track.isMuted) {
			track.voice->Trigger(48, 1.0, 0);
			if (track.muteGroupId) {
				for (auto& other : d_tracks) {
					if (&track != &other &&
						other.muteGroupId &&
						other.muteGroupId.value() == track.muteGroupId.value()) {
						other.voice->Stop(); }}}}}}


}  // namespace ralm
}  // namespace rqdq
