#include "src/ral/ralm/ralm_grid_sequencer.hxx"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "src/ral/raldsp/raldsp_sampler.hxx"

namespace rqdq {
namespace ralm {

GridSequencer::GridSequencer() {
	SetSwing(50); }


void GridSequencer::SetTempo(const int bpm) {
	double ax = 44100.0 / (bpm/600.0);
	for (int i=0; i<kPPQ; i++) {
		double chunk = ax / (kPPQ-i);
		int ichunk = lround(chunk);
		ax = ax - ichunk;
		d_ppqLUT[i] = ichunk; }
	d_tempoInBPM = bpm; }


int GridSequencer::GetTempo() const {
	return d_tempoInBPM; }


void GridSequencer::SetSwing(int pct) {
	pct = std::clamp(pct, 50, 75);
	const int eighthNoteInPPQ{48};
	int first = lround(eighthNoteInPPQ * pct / 100.0);
	int second = eighthNoteInPPQ - first;
	d_swing = { first, second };
	d_swingPct = pct; }


int GridSequencer::GetSwing() const {
	return d_swingPct; }


void GridSequencer::AddTrack(raldsp::SingleSampler& voice, std::optional<int> muteGroupId) {
	d_tracks.emplace_back(&voice, muteGroupId); }


void GridSequencer::IncrementT() {
	d_t++;
	d_ppqStamp++; }


void GridSequencer::Rewind() {
	d_gridPos = 0;
	d_t = 0; }


bool GridSequencer::Update() {
	bool changed = false;
	if (d_state==PlayerState::Stopped && d_nextState==PlayerState::Playing) {
		changed = true;
		d_state = PlayerState::Playing;
		d_sampleCounter = 0;
		d_ppqCounter = 1;
		d_gridPos = -1;
		d_t = -1;  // rewind
		d_ppqStamp = 0; }
	if (d_state==PlayerState::Playing && d_nextState==PlayerState::Stopped) {
		changed = true;
		//std::cout << "[->STOPPED]" << std::flush;
		d_state = PlayerState::Stopped;
		d_ppqStamp = 0; }
	return changed; }


bool GridSequencer::Process() {
	bool tracksWillUpdate = false;
	if (d_state == PlayerState::Playing) {
		d_sampleCounter--;
		if (d_sampleCounter <= 0) {
			IncrementT();
			d_ppqCounter--;
			if (d_ppqCounter == 0) {
				d_gridPos++;
				d_ppqCounter = d_swing[d_gridPos%2];
				tracksWillUpdate = true; }
			d_sampleCounter = d_ppqLUT[d_t%kPPQ];
			if (d_t >= (d_patternLength/4)*kPPQ) {
				// end of pattern
				Rewind(); }}}

	bool updated = false;
	if (tracksWillUpdate) {
		d_lastPlayedGridPosition = d_gridPos;
		updated = true;
		for (auto& track : d_tracks) {
			if (track.grid[d_gridPos] != 0 && !track.isMuted) {
				track.voice->Trigger(48, 1.0, 0);
				if (track.muteGroupId) {
					for (auto& other : d_tracks) {
						if (&track != &other &&
							other.muteGroupId &&
							other.muteGroupId.value() == track.muteGroupId.value()) {
							other.voice->Stop(); }}}}}}
	return updated; }


}  // namespace ralm
}  // namespace rqdq
