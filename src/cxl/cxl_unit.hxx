#pragma once
#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_sampler.hxx"
#include "src/ral/ralm/ralm_grid_sequencer.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"
#include "src/rcl/rcls/rcls_file.hxx"

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

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

	// sampler voices
	void IncrementKit();
	void DecrementKit();
	void SaveKit();
	void LoadKit();
	const std::string GetVoiceParameterName(int track, int num);
	const std::string& GetWaveName(int waveId);
	int GetVoiceParameterValue(int track, int num);
	void Adjust(int track, int param, int amt);

    // audio render
    void Render(float* left, float* right, int numSamples);

private:
	template<typename T>
	void Adjust2(T& slot, T lower, T upper, T amt) {
		T oldValue = slot;
		T newValue = std::clamp(oldValue+amt, lower, upper);
		if (oldValue != newValue) {
			slot = newValue;
			if (d_updateFunc) {
				d_updateFunc(); }}}

public:
	std::function<void()> d_updateFunc;
private:
	int d_kitNum = 0;
	int d_patternNum = 0;
	ralw::WaveTable d_waveTable;
	raldsp::BasicMixer d_mixer;
	ralm::GridSequencer d_gridSequencer;
	std::vector<raldsp::SingleSampler> d_voices; };


}  // namespace cxl
}  // namespace rqdq
