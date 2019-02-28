#include "src/ral/ralm/ralm_grid_sequencer.hxx"

#include "src/ral/raldsp/raldsp_sampler.hxx"

#include <iostream>

namespace rqdq {
namespace ralm {

void GridSequencer::SetTempo(const int bpm) {
	double ax = 44100.0 / (bpm/600.0);
	for (int i=0; i<kPPQ; i++) {
		double chunk = ax / (kPPQ-i);
		int ichunk = int(chunk + 0.5);
		ax = ax - ichunk;
		d_ppqLUT[i] = ichunk; }
	d_tempoInBPM = bpm; }


int GridSequencer::GetTempo() {
	return d_tempoInBPM; }


void GridSequencer::AddTrack(raldsp::SingleSampler& voice) {
	d_tracks.emplace_back(GridTrack{ &voice }); }


void GridSequencer::IncrementT() {
	d_t++;
	d_ppqStamp++; }


void GridSequencer::Rewind() {
	d_t = 0; }


bool GridSequencer::Update() {
	bool changed = false;
	if (d_state==PlayerState::Stopped && d_nextState==PlayerState::Playing) {
		changed = true;
		d_state = PlayerState::Playing;
		d_sampleCounter = 0;
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
			tracksWillUpdate = true;
			d_sampleCounter = d_ppqLUT[d_t%kPPQ];
			if (d_t >= (d_patternLength/4)*kPPQ) {
				// end of pattern
				Rewind(); }}}

	bool updated = false;
	if (tracksWillUpdate && (d_t % (kPPQ/kBeatsPerBar) == 0)) {
		const int gridPos = d_t / (kPPQ/kBeatsPerBar);
		d_lastPlayedGridPosition = gridPos;
		updated = true;
		for (auto& track : d_tracks) {
			if (track.grid[gridPos] != 0 && !track.isMuted) {
				track.voice->Trigger(48, 1.0, 0); }}}
	return updated; }


}  // namespace ralm
}  // namespace rqdq
