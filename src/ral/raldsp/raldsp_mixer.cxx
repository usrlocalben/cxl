#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_idspoutput.hxx"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rqdq {
namespace raldsp {


void BasicMixer::AddChannel(IDSPOutput& input, float gain) {
	auto match = std::find_if(begin(d_channels), end(d_channels),
	                          [&](const auto& item) { return &item.input == &input; });
	if (match != d_channels.end()) {
		throw new std::runtime_error("attempted to add same input twice to mixer"); }

	d_channels.emplace_back(BasicMixerChannel{ input, gain, false }); }


void BasicMixer::Update(int tempo) {
	for (auto& channel : d_channels) {
		channel.input.Update(tempo); }}


void BasicMixer::Process(float* inputs, float* outputs) {
	float& c0 = outputs[0];
	float& c1 = outputs[1];
	c0 = 0, c1 = 0;
	for (auto& channel : d_channels) {
		std::array<float, 2> sub;
		channel.input.Process(nullptr, sub.data());
		if (!channel.mute) {
			c0 += sub[0] * channel.gain;
			c1 += sub[1] * channel.gain; }}}


}  // namespace raldsp
}  // namespace rqdq

