#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rqdq {
namespace {

class PanningLUT {
public:
	PanningLUT() {
		for (int i=0; i<101; i++) {
			left[i] = sqrt( (101-i)/101.0 );
			right[i] = sqrt(      i /101.0 );
		}}
private:
	std::array<double, 101> left;
	std::array<double, 101> right;
};

PanningLUT panningLUT;

}  // namespace
namespace raldsp {


void BasicMixerChannel::Update(int tempo) {}


void BasicMixerChannel::Process(float* inputs, float* outputs) {
	outputs[0] = inputs[0] * (d_gain / float(100.0));
	outputs[1] = inputs[0] * (d_gain / float(100.0)); }


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
