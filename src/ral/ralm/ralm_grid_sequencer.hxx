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
private:
	void TriggerCurrentNote();

public:
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
	int GetPlayingNoteIndex() const { return d_noteIdx; }

private:
	std::vector<GridTrack> d_tracks;
	int d_patternLength{16};
	int d_tempoInBPM{1200};
	int d_swingPct{50};

	// playing state
	PlayerState d_state{PlayerState::Stopped}, d_nextState{PlayerState::Stopped};
	int d_pointTimeRemainingInSamples{0};
	int d_noteTimeRemainingInPoints{0};
	double d_pointSamplesError{0};
	int d_noteIdx{0};
	int d_timeInPoints{0}; };


}  // close package namespace
}  // close enterprise namespace
