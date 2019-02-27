#pragma once
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"

#include <stdexcept>
#include <vector>

namespace rqdq {
namespace raldsp {


struct BasicMixerChannel {
	void Update(int tempo);
	void Process(float* inputs, float* outputs);
	void Initialize() {
		d_gain = 100;
		d_mute = false; }
	int d_gain = 100;
	bool d_mute = false; };


class BasicMixer : public IAudioDevice {
	// IAudioDevice
public:
	void Update(int tempo) override;
	void Process(float*, float*) override;

public:
	void AddChannel();
	int GetNumChannels() { return d_channels.size(); }
	void SetChannelGain(int ch, float value) {
		EnsureValidChannelId(ch);
		d_channels[ch].d_gain = value; }
	void SetChannelMute(int ch, bool value) {
		EnsureValidChannelId(ch);
		d_channels[ch].d_mute = value; }
	float GetChannelGain(int ch) {
		EnsureValidChannelId(ch);
		return d_channels[ch].d_gain; }
	bool GetChannelMute(int ch) {
		EnsureValidChannelId(ch);
		return d_channels[ch].d_mute; }

private:
	void EnsureValidChannelId(int ch) {
		if (!(0<=ch && ch<GetNumChannels())) {
			throw new std::runtime_error("invalid channel id"); }}

public:
	std::vector<BasicMixerChannel> d_channels; };


}  // close package namespace
}  // close enterprise namespace
