#pragma once
#include <array>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "src/ral/raldsp/raldsp_distortion.hxx"
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"

namespace rqdq {
namespace raldsp {

template <typename CHANNEL_STRIP>
class BasicMixer : public IAudioDevice {
	// IAudioDevice
public:
	void Update(int tempo) override {}

	void Process(float* inputs, float* outputs) override {
		std::array<float, 6> sums{};
		for (int i=0; i<d_channels.size(); i++) {
			std::array<float, 6> channelOut;
			d_channels[i].Process(&(inputs[i]), channelOut.data());
			for (int oi=0; oi<6; oi++) {
				sums[oi] += channelOut[oi]; }}

		// std::array<float, 2> fxOut;
		// d_delay.Process(&sums[2], fxOut.data());
		// sums[0] += fxOut[0];
		// sums[1] += fxOut[1];
		// d_reverb.Process(&sums[4], fxOut.data());
		// sums[0] += fxOut[0];
		// sums[1] += fxOut[1];

		outputs[0] = sums[0];
		outputs[1] = sums[1]; }

public:
	void AddChannel() {
		d_channels.emplace_back(); }

	int GetNumChannels() const { return static_cast<int>(d_channels.size()); }

	CHANNEL_STRIP& operator[](int ch) {
		EnsureValidChannelId(ch);
		return d_channels[ch]; }

	const CHANNEL_STRIP& operator[](int ch) const {
		EnsureValidChannelId(ch);
		return d_channels[ch]; }

private:
	void EnsureValidChannelId(int ch) const {
		if (!(0<=ch && ch<GetNumChannels())) {
			throw new std::runtime_error("invalid channel id"); }}

public:
	std::vector<CHANNEL_STRIP> d_channels; };


}  // close package namespace
}  // close enterprise namespace
