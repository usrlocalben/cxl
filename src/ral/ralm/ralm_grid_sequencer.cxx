#include "src/ral/ralm/ralm_grid_sequencer.hxx"

#include "src/ral/raldsp/raldsp_sampler.hxx"

#include <iostream>

namespace rqdq {
namespace ralm {

void GridSequencer::SetTempo(const int bpm) {
	double ax = 44100.0 / (bpm/600.0);
	for (int i=0; i<96; i++) {
		double chunk = ax / (96-i);
		int ichunk = int(chunk + 0.5);
		ax = ax - ichunk;
		d_ppqLUT[i] = ichunk; }
	d_tempoInBPM = bpm; }


void GridSequencer::AddTrack(raldsp::SingleSampler& voice) {
	d_tracks.emplace_back(GridTrack{ &voice }); }


void GridSequencer::IncrementT() {
	d_t++;
	d_ppqStamp++; }


void GridSequencer::Rewind() {
	d_t = 0; }


void GridSequencer::Update() {
	//std::cout << "U" << std::flush;
	//d_tracksWillUpdate = false;

	if (d_state==PlayerState::Stopped && d_nextState==PlayerState::Playing) {
		std::cout << "[->PLAYING]" << std::flush;
		d_state = PlayerState::Playing;
		//d_sampleCounter = d_ppqLUT[d_t%96];
		//d_tracksWillUpdate = true;
		d_sampleCounter = 0;
		d_t = -1;  // rewind
		d_ppqStamp = 0; }
	if (d_state==PlayerState::Playing && d_nextState==PlayerState::Stopped) {
		std::cout << "[->STOPPED]" << std::flush;
		d_state = PlayerState::Stopped;
		d_ppqStamp = 0; }}


void GridSequencer::Tick() {
	d_tracksWillUpdate = false;
	if (d_state == PlayerState::Playing) {
		//std::cout << "[TP]" << std::flush;
		d_sampleCounter--;
		if (d_sampleCounter <= 0) {
			IncrementT();
			//std::cout << "+" << std::flush;
			d_tracksWillUpdate = true;
			d_sampleCounter = d_ppqLUT[d_t%96];
			//std::cout <<"[cnt=" << d_sampleCounter << "]" << std::flush;

			if (d_t >= 1*4*96) {
				// end of pattern
				Rewind(); }}}

	if (!d_tracksWillUpdate) return;
	if (d_t % 24 != 0) return;

	//std::cout << "!" << std::flush;

	const int gridPos = d_t / 24;
	//std::cout << "[GP=" << gridPos << "]" << std::flush;
	for (auto& track : d_tracks) {
		if (track.grid[gridPos]) {
			//std::cout << "T" << std::flush;
			track.voice->Trigger(36, 1.0, 0); }}}


}  // close package namespace
}  // close enterprise namespace
