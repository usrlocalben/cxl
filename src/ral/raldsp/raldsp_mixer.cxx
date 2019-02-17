#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_idspoutput.hxx"

#include <algorithm>
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


std::pair<float, float> BasicMixer::GetNextSample() {
	float c0 = 0;
	float c1 = 0;
	for (auto& channel : d_channels) {
		auto[sub0, sub1] = channel.input.GetNextSample();
		if (!channel.mute) {
			c0 += sub0 * channel.gain;
			c1 += sub1 * channel.gain; }}
	return {c0, c1};}


}  // namespace raldsp
}  // namespace rqdq

