#pragma once
#include <functional>
#include <memory>
#include <string_view>

namespace rqdq {
namespace cxl {

class CXLUnit {
	class impl;
public:
	CXLUnit();
	~CXLUnit();

	CXLUnit(CXLUnit&& /*unused*/);
	CXLUnit& operator=(CXLUnit&& /*unused*/);
	CXLUnit(const CXLUnit&) = delete;
	CXLUnit& operator=(const CXLUnit&) = delete;

	bool IsLoading() const;
	float GetLoadingProgress() const;
	std::string_view GetLoadingName() const;

	void Play();
	void Stop();
	bool IsPlaying() const;
	void SetTempo(int /*value*/);
	int GetTempo() const;
	void SetSwing(int /*pct*/);
	int GetSwing() const;
	void Trigger(int /*track*/);

	int GetCurrentKitNumber() const;
	int GetCurrentPatternNumber() const;
	void SetPatternLength(int /*value*/);
	int GetPatternLength() const;
	bool IsTrackMuted(int /*ti*/) const;
	void ToggleTrackMute(int /*ti*/);
	void CommitPattern();
	void SaveKit();
	void LoadKit();
	void InitializeKit();
	void IncrementKit();
	void DecrementKit();
	void ToggleTrackGridNote(int /*track*/, int /*pos*/);
	int GetTrackGridNote(int /*track*/, int /*pos*/) const;
	void SetTrackGridNote(int /*track*/, int /*pos*/, int /*note*/);
	void SwitchPattern(int /*pid*/);

	std::string_view GetVoiceParameterName(int ti, int pi) const;
	int GetVoiceParameterValue(int ti, int pi) const;
	void AdjustVoiceParameter(int ti, int pi, int offset);
	std::string_view GetEffectParameterName(int ti, int pi) const;
	int GetEffectParameterValue(int ti, int pi) const;
	void AdjustEffectParameter(int ti, int pi, int offset);
	std::string_view GetMixParameterName(int ti, int pi) const;
	int GetMixParameterValue(int ti, int pi) const;
	void AdjustMixParameter(int ti, int pi, int offset);

	int ConnectPlaybackPositionChanged(std::function<void(int)> fn);
	int ConnectPlaybackStateChanged(std::function<void(bool)> fn);
	int ConnectLoaderStateChanged(std::function<void()> fn);
	void DisconnectPlaybackPositionChanged(int id);
	void DisconnectPlaybackStateChanged(int id);
	void DisconnectLoaderStateChanged(int id);

    void Render(float* /*left*/, float* /*right*/, int /*numSamples*/);
	int GetPlayingNoteIndex() const;

	std::string_view GetWaveName(int /*waveId*/) const;

private:
	std::unique_ptr<impl> d_impl; };


}  // namespace cxl
}  // namespace rqdq
