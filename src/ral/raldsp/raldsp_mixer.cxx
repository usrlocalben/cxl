#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rqdq {
namespace {

class PanningLUT {
public:
	PanningLUT() {
		for (int i=0; i<128; i++) {
			leftGain[i]  = sqrt((128-i)/128.0);
			rightGain[i] = sqrt(     i /128.0); }}

	void Pan(float* inputs, float* outputs, int setting) {
		assert(-64 <= setting && setting <= 63);
		outputs[0] = inputs[0] * leftGain[setting+64];
		outputs[1] = inputs[0] * rightGain[setting+64]; }

private:
	std::array<float, 128> leftGain;
	std::array<float, 128> rightGain; };

PanningLUT panningLUT;

}  // namespace
namespace raldsp {


void BasicMixerChannel::Update(int tempo) {}


void BasicMixerChannel::Process(float* inputs, float* outputs) {
	float& in = inputs[0];
	float tmp = 0;

	d_distortor.d_threshold = 127 - d_distortion;
	d_distortor.Process(inputs, &tmp);
	inputs[0] = tmp;

	in *= (d_gain / float(100.0));
	panningLUT.Pan(&in, outputs, d_pan); }


void BasicMixer::AddChannel() {
	d_channels.emplace_back(BasicMixerChannel{}); }


void BasicMixer::Update(int tempo) {}


void BasicMixer::Process(float* inputs, float* outputs) {
	float& sumLeft = outputs[0];
	float& sumRight = outputs[1];
	sumLeft = 0;
	sumRight = 0;
	for (int i=0; i<d_channels.size(); i++) {
		float sub[2];
		d_channels[i].Process(&(inputs[i]), sub);
		sumLeft  += sub[0];
		sumRight += sub[1]; }}


}  // namespace raldsp
}  // namespace rqdq
