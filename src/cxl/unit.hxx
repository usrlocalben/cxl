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

	CXLUnit(CXLUnit&&);
	CXLUnit& operator=(CXLUnit&&);
	CXLUnit(const CXLUnit&) = delete;
	CXLUnit& operator=(const CXLUnit&) = delete;

	bool IsLoading() const;
	float GetLoadingProgress() const;
	std::string_view GetLoadingName() const;

	void Play();
	void Stop();
	bool IsPlaying() const;
	void SetTempo(int);
	int GetTempo() const;
	void SetSwing(int);
	int GetSwing() const;
	void Trigger(int);

	int GetCurrentKitNumber() const;
	int GetCurrentPatternNumber() const;
	void SetPatternLength(int);
	int GetPatternLength() const;
	bool IsTrackMuted(int) const;
	void ToggleTrackMute(int);
	void CommitPattern();
	void SaveKit();
	void LoadKit();
	void InitializeKit();
	void IncrementKit();
	void DecrementKit();
	void ToggleTrackGridNote(int, int);
	int GetTrackGridNote(int, int) const;
	void SetTrackGridNote(int, int, int);
	void SwitchPattern(int);

	std::string_view GetVoiceParameterName(int ti, int pi) const;
	int GetVoiceParameterValue(int ti, int pi) const;
	void AdjustVoiceParameter(int ti, int pi, int offset);
	std::string_view GetEffectParameterName(int ti, int pi) const;
	int GetEffectParameterValue(int ti, int pi) const;
	void AdjustEffectParameter(int ti, int pi, int offset);
	std::string_view GetMixParameterName(int ti, int pi) const;
	int GetMixParameterValue(int ti, int pi) const;
	void AdjustMixParameter(int ti, int pi, int offset);

	void ConnectPlaybackPositionChanged(std::function<void(int)> fn);
	void ConnectPlaybackStateChanged(std::function<void(bool)> fn);
	void ConnectLoaderStateChanged(std::function<void()> fn);

    void Render(float*, float*, int);
	int GetPlayingNoteIndex() const;

	std::string_view GetWaveName(int) const;

private:
	std::unique_ptr<impl> d_impl; };


}  // namespace cxl
}  // namespace rqdq
