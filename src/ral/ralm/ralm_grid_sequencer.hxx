#pragma once
#include "src/ral/raldsp/raldsp_sampler.hxx"

#include <array>
#include <cassert>
#include <stdexcept>
#include <vector>

namespace rqdq {
namespace ralm {

struct GridTrack {
	raldsp::SingleSampler* voice;
	std::array<int, 16> grid = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; };


class PlayerState {
public:
	enum Enumeration {
		Stopped = 0,
		Precounting = 1,
		Playing = 2,
		Unknown };
	PlayerState() :e(Unknown) {}
	PlayerState(Enumeration state) :e(state) {}
	operator Enumeration() const { assert(e!=Unknown); return e; }
private:
	Enumeration e; };


class GridSequencer {
public:
	void Play() {
		if (d_state != PlayerState::Stopped) {
			return; }
		d_nextState = PlayerState::Playing; }

	void Stop() {
		if (d_state != PlayerState::Playing) {
			return; }
		d_nextState = PlayerState::Stopped; }

	void Update();
	void Tick();

	void SetTempo(int bpm);
	void AddTrack(raldsp::SingleSampler& voice);
	void SetTrackGridNote(int tn, int pos, int value) {
		if (!(0<=tn && tn<d_tracks.size())) {
			throw std::runtime_error("invalid track"); }
		d_tracks[tn].grid[pos] = value; }
	int GetTrackGridNote(int tn, int pos) {
		if (!(0<=tn && tn<d_tracks.size())) {
			throw std::runtime_error("invalid track"); }
		return d_tracks[tn].grid[pos]; }
	void ToggleTrackGridNote(int tn, int pos) {
		if (!(0<=tn && tn<d_tracks.size())) {
			throw std::runtime_error("invalid track"); }
		auto& cell = d_tracks[tn].grid[pos];
		cell = (cell==1?0:1); }

private:
	void IncrementT();
	void Rewind();
	std::vector<GridTrack> d_tracks;

	// playing state
	int d_tempoInBPM = 1200;
	PlayerState d_state{PlayerState::Stopped}, d_nextState;
	int d_sampleCounter = 0;
	bool d_tracksWillUpdate = false;
	int d_t = 0;
	int d_ppqStamp = 0;
	std::array<int, 96> d_ppqLUT; };


}  // close package namespace
}  // close enterprise namespace
