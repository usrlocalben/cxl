#pragma once
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"
#include "src/ral/raldsp/raldsp_distortion.hxx"

#include <stdexcept>
#include <vector>

namespace rqdq {
namespace raldsp {


struct BasicMixerChannel {
	void Update(int tempo);
	void Process(float* inputs, float* outputs);
	void Initialize() {
		d_gain = 100;
		d_mute = false;
		d_pan = 0;
		d_distortion = 0;
		d_send1 = 0;
		d_send2 = 0; }
	int d_gain = 100;
	bool d_mute = false;
	int d_pan =0;
	int d_distortion = 0;
	int d_send1 = 0;
	int d_send2 = 0;

	Distortor d_distortor{1}; };


class BasicMixer : public IAudioDevice {
	// IAudioDevice
public:
	void Update(int tempo) override;
	void Process(float*, float*) override;

public:
	void AddChannel();
	int GetNumChannels() { return static_cast<int>(d_channels.size()); }
	void SetChannelGain(int ch, int value) {
		EnsureValidChannelId(ch);
		d_channels[ch].d_gain = value; }
	void SetChannelPan(int ch, int value) {
		EnsureValidChannelId(ch);
		d_channels[ch].d_pan = value; }
	void SetChannelMute(int ch, bool value) {
		EnsureValidChannelId(ch);
		d_channels[ch].d_mute = value; }
	int GetChannelGain(int ch) {
		EnsureValidChannelId(ch);
		return d_channels[ch].d_gain; }
	int GetChannelPan(int ch) {
		EnsureValidChannelId(ch);
		return d_channels[ch].d_pan; }
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
