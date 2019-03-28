#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <optional>
#include <stdexcept>
#include <vector>

#include "src/ral/raldsp/raldsp_sampler.hxx"

namespace rqdq {
namespace cxl {

constexpr int kPPQ = 96;
constexpr int kBeatsPerBar = 4;


class GridSequencer {
	struct Track {
		Track(raldsp::SingleSampler* voice, std::optional<int> muteGroupId)
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
public:
	GridSequencer();

	void Play() {
		if (state_ != PlayerState::Stopped) {
			return; }
		wantedState_ = PlayerState::Playing; }

	void Stop() {
		if (state_ != PlayerState::Playing) {
			return; }
		wantedState_ = PlayerState::Stopped; }

	bool Update();
	bool Process();
private:
	void TriggerCurrentNote();

public:
	bool IsPlaying() const { return state_ == PlayerState::Playing; }

	void SetTempo(int bpm);
	int GetTempo() const;
	void SetSwing(int pct);
	int GetSwing() const;
	void AddTrack(raldsp::SingleSampler&, std::optional<int>);

	int GetPatternLength() const { return patternLength_; }
	void SetPatternLength(int value) {
		if (!(value==16 || value==32 || value==64)) {
			throw std::runtime_error("invalid pattern length"); }
		patternLength_ = value; }

	void SetTrackGridNote(int tn, int pos, int value) {
		if (!(0<=tn && tn<tracks_.size())) {
			throw std::runtime_error("invalid track"); }
		tracks_[tn].grid[pos] = value; }
	int GetTrackGridNote(int tn, int pos) const {
		if (!(0<=tn && tn<tracks_.size())) {
			throw std::runtime_error("invalid track"); }
		return tracks_[tn].grid[pos]; }
	void ToggleTrackGridNote(int tn, int pos) {
		if (!(0<=tn && tn<tracks_.size())) {
			throw std::runtime_error("invalid track"); }
		auto& cell = tracks_[tn].grid[pos];
		cell = (cell==1?0:1); }
	void ClearTrackGrid(int tn) {
		tracks_[tn].grid.fill(0); }
	bool IsTrackMuted(int tn) const {
		return tracks_[tn].isMuted; }
	void ToggleTrackMute(int tn) {
		tracks_[tn].isMuted = !tracks_[tn].isMuted; }

	void InitializePattern() {
		std::for_each(begin(tracks_), end(tracks_),
		              [](auto& item) { item.grid.fill(0); }); }
	int GetPlayingNoteIndex() const { return noteIdx_; }

private:
	std::vector<Track> tracks_;
	int patternLength_{16};
	int tempoInBPM_{1200};
	int swingPct_{50};

	// playing state
	PlayerState state_{PlayerState::Stopped}, wantedState_{PlayerState::Stopped};
	int pointTimeRemainingInSamples_{0};
	int noteTimeRemainingInPoints_{0};
	double pointSamplesError_{0};
	int noteIdx_{0};
	int timeInPoints_{0}; };


}  // close package namespace
}  // close enterprise namespace
