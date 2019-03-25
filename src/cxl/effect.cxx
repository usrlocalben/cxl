#include "effect.hxx"

#include <algorithm>
#include <cmath>

namespace rqdq {
namespace {

constexpr float kEQQ{4.0F};


}  // namespace

namespace cxl {

CXLEffects::CXLEffects() = default;


void CXLEffects::Update(int tempo) {
	const int freq = d_lowpassFreq;
	const int q = d_lowpassQ;
	if (freq == 127) {
		d_filter.SetBypass(true); }
	else {
		d_filter.SetBypass(false);
		d_filter.SetCutoff(pow(freq/127.0F, 2.0F));
		// scale Q-max slightly under 1.0
		d_filter.SetQ(sqrt(q/128.9F)); }

	d_filter.Update(tempo);

	d_delay.SetTime(d_delayTime);
	d_delay.SetFeedbackGain(d_delayFeedback / 127.0);
	d_delay.Update(tempo);

	d_reducer.d_midi = d_reduce;
	d_reducer.Update(tempo);

	if (d_eqGain != d_eqGain2 || d_eqCenter != d_eqCenter2) {
		d_eqGain2 = d_eqGain;
		d_eqCenter2 = d_eqCenter;
		float gain = std::clamp(d_eqGain, -64, 63) / 63.0 * 12;
		float freq = pow(std::clamp(d_eqCenter, 0, 127) / 127.0, 2.0) * 8000.0;
		d_eq.Configure(raldsp::MultiModeFilter::Mode::PEQ,
		               gain,
		               freq,
		               kEQQ,
		               raldsp::MultiModeFilter::ParamType::Q,
		               44100.0); }}


void CXLEffects::Process(float* inputs, float* outputs) {
	inputs[0] = d_eq.Process(inputs[0]);
	float filtered;
	d_filter.Process(inputs, &filtered);

	float delaySend = d_delaySend / 127.0;
	float toDelay = filtered * delaySend;
	float fromDelay;
	d_delay.Process(&toDelay, &fromDelay);

	float filteredPlusDelay = filtered + fromDelay;
	d_reducer.Process(&filteredPlusDelay, outputs); }


}  // namespace cxl
}  // namespace rqdq
