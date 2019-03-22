#pragma once
#include <array>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "src/ral/raldsp/raldsp_distortion.hxx"
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"

#include "3rdparty/freeverb/freeverb.hxx"

namespace rqdq {
namespace raldsp {

template <typename CHANNEL_STRIP>
class BasicMixer : public IAudioDevice {
	// IAudioDevice
public:
	void Update(int tempo) override {
		d_reverb.setroomsize(0.5);
		d_reverb.setdamp(0.5);
		d_reverb.setwidth(0.5); }

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
		std::array<float, 2> tmp{};
		d_reverb.processmix(&sums[4], &sums[5], &tmp[0], &tmp[1], 1, 0);
		sums[0] += tmp[0];
		sums[1] += tmp[1];

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
	std::vector<CHANNEL_STRIP> d_channels;
	Freeverb d_reverb{}; };


}  // close package namespace
}  // close enterprise namespace
