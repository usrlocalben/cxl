#pragma once
#include "src/ral/raldsp/raldsp_idspoutput.hxx"

#include <stdexcept>
#include <vector>

namespace rqdq {
namespace raldsp {


struct BasicMixerChannel {
	raldsp::IDSPOutput& input;
	float gain = 1.0f;
	bool mute = false; };


class BasicMixer : public IDSPOutput {
	// IDSPOutput
public:
	void Update(int tempo) override;
	void Process(float*, float*) override;

public:
	void AddChannel(IDSPOutput& input, float gain=1.0f);
	int GetNumChannels() { return d_channels.size(); }
	void SetChannelGain(int ch, float value) {
		EnsureValidChannelId(ch);
		d_channels[ch].gain = value; }
	void SetChannelMute(int ch, bool value) {
		EnsureValidChannelId(ch);
		d_channels[ch].gain = value; }
	float GetChannelGain(int ch) {
		EnsureValidChannelId(ch);
		return d_channels[ch].gain; }
	bool GetChannelMute(int ch) {
		EnsureValidChannelId(ch);
		return d_channels[ch].mute; }

private:
	void EnsureValidChannelId(int ch) {
		if (!(0<=ch && ch<GetNumChannels())) {
			throw new std::runtime_error("invalid channel id"); }}

private:
	std::vector<BasicMixerChannel> d_channels; };


}  // close package namespace
}  // close enterprise namespace
