#pragma once
#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_sampler.hxx"
#include "src/ral/ralm/ralm_grid_sequencer.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"
#include "src/rcl/rcls/rcls_file.hxx"

#include <algorithm>
#include <string>
#include <vector>
#include "3rdparty/wink/wink/signal.hpp"
#include "3rdparty/wink/wink/slot.hpp"

namespace rqdq {
namespace cxl {


class CXLUnit {
public:
	CXLUnit();

	// transport controls
	void Play();
	void Stop();
	bool IsPlaying();
	int GetLastPlayedGridPosition();
	void SetTempo(int value);
	int GetTempo();

	// pattern editing
	void ToggleTrackGridNote(int track, int pos);
	int GetTrackGridNote(int track, int pos);
	void SwitchPattern(int pid);
	void CommitPattern();
	int GetCurrentPatternNumber() { return d_patternNum; }
	int GetPatternLength() { return d_sequencer.GetPatternLength(); }
	void SetPatternLength(int value) { d_sequencer.SetPatternLength(value); }

	// sampler voices
	void InitializeKit();
	void IncrementKit();
	void DecrementKit();
	void SaveKit();
	void LoadKit();
	void SwitchKit(int n);
	int GetCurrentKitNumber() { return d_kitNum; }
	const std::string& GetCurrentKitName() { return d_kitName; }
	const std::string GetVoiceParameterName(int ti, int pi);
	int GetVoiceParameterValue(int ti, int pi);
	void AdjustVoiceParameter(int ti, int pi, int offset);
	const std::string& GetWaveName(int waveId);
	void Trigger(int track);

    // audio render
    void Render(float* left, float* right, int numSamples);

private:
	template<typename T>
	void Adjust2(int id, T& slot, T lower, T upper, T amt) {
		T oldValue = slot;
		T newValue = std::clamp(oldValue+amt, lower, upper);
		if (oldValue != newValue) {
			slot = newValue;
			d_voiceParameterChanged.emit(id);}}

public:
	wink::signal<wink::slot<void(int)>> d_voiceParameterChanged;
	wink::signal<wink::slot<void(int)>> d_tempoChanged;
	wink::signal<wink::slot<void(int)>> d_patternDataChanged;
	wink::signal<wink::slot<void(bool)>> d_playbackStateChanged;
	wink::signal<wink::slot<void(int)>> d_playbackPositionChanged;
	wink::signal<wink::slot<void()>> d_currentPatternChanged;

	// BEGIN wave-table loader
private:
	bool d_loading = false;
	int d_nextFileId = 0;
	int d_nextWaveId = 1;
	std::vector<std::string> d_filesToLoad;
public:
	wink::signal<wink::slot<void()>> d_loaderStateChanged;
	float GetLoadingProgress();
	const std::string GetLoadingName() {
		if (d_nextFileId < d_filesToLoad.size()) {
			return d_filesToLoad[d_nextFileId]; }
		else {
			return "";}}
	bool IsLoading() { return d_loading; }
private:
	void BeginLoadingWaves();
	void MakeProgressLoadingWaves();
	void onWaveIOComplete(const std::vector<uint8_t>& data);
	void onWaveIOError(int error);
	//  END  wave-table loader

	int d_kitNum = 0;
	std::string d_kitName = "new kit";
	int d_patternNum = 0;
	ralw::WaveTable d_waveTable;
	raldsp::BasicMixer d_mixer;
	ralm::GridSequencer d_sequencer;
	std::vector<raldsp::SingleSampler> d_voices; };


}  // namespace cxl
}  // namespace rqdq
