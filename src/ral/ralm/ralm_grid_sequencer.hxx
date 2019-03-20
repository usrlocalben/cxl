#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <optional>
#include <stdexcept>
#include <vector>

#include "src/ral/raldsp/raldsp_sampler.hxx"

namespace rqdq {
namespace ralm {

constexpr int kPPQ = 96;
constexpr int kBeatsPerBar = 4;

struct GridTrack {
	GridTrack(raldsp::SingleSampler* voice, std::optional<int> muteGroupId)
		:voice(voice), muteGroupId(muteGroupId) {};

	raldsp::SingleSampler* voice = nullptr;
	bool isMuted = false;
	std::array<int, 16*4> grid{};
	std::optional<int> muteGroupId{}; };


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
	GridSequencer();

	void Play() {
		if (d_state != PlayerState::Stopped) {
			return; }
		d_nextState = PlayerState::Playing; }

	void Stop() {
		if (d_state != PlayerState::Playing) {
			return; }
		d_nextState = PlayerState::Stopped; }

	bool Update();
	bool Process();

	bool IsPlaying() const { return d_state == PlayerState::Playing; }

	void SetTempo(int bpm);
	int GetTempo() const;
	void SetSwing(int pct);
	int GetSwing() const;
	void AddTrack(raldsp::SingleSampler&, std::optional<int>);

	int GetPatternLength() const { return d_patternLength; }
	void SetPatternLength(int value) {
		if (!(value==16 || value==32 || value==64)) {
			throw std::runtime_error("invalid pattern length"); }
		d_patternLength = value; }

	void SetTrackGridNote(int tn, int pos, int value) {
		if (!(0<=tn && tn<d_tracks.size())) {
			throw std::runtime_error("invalid track"); }
		d_tracks[tn].grid[pos] = value; }
	int GetTrackGridNote(int tn, int pos) const {
		if (!(0<=tn && tn<d_tracks.size())) {
			throw std::runtime_error("invalid track"); }
		return d_tracks[tn].grid[pos]; }
	void ToggleTrackGridNote(int tn, int pos) {
		if (!(0<=tn && tn<d_tracks.size())) {
			throw std::runtime_error("invalid track"); }
		auto& cell = d_tracks[tn].grid[pos];
		cell = (cell==1?0:1); }
	void ClearTrackGrid(int tn) {
		d_tracks[tn].grid.fill(0); }
	bool IsTrackMuted(int tn) const {
		return d_tracks[tn].isMuted; }
	void ToggleTrackMute(int tn) {
		d_tracks[tn].isMuted = !d_tracks[tn].isMuted; }

	void InitializePattern() {
		std::for_each(begin(d_tracks), end(d_tracks),
		              [](auto& item) { item.grid.fill(0); }); }
	int GetLastPlayedGridPosition() const { return d_lastPlayedGridPosition; }

private:
	void IncrementT();
	void Rewind();
	std::vector<GridTrack> d_tracks;
	int d_patternLength = 16;

	// playing state
	int d_tempoInBPM = 1200;
	int d_swingPct{50};
	std::array<int, 2> d_swing{{ 24, 24 }};
	PlayerState d_state{PlayerState::Stopped}, d_nextState{PlayerState::Stopped};
	int d_sampleCounter = 0;
	int d_ppqCounter{0};
	int d_t = 0;
	int d_gridPos{0};
	int d_ppqStamp = 0;
	int d_lastPlayedGridPosition = 0;
	std::array<int, 96> d_ppqLUT; };


}  // close package namespace
}  // close enterprise namespace
