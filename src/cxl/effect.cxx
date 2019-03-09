#include "src/cxl/effect.hxx"

#include <cmath>

namespace rqdq {
namespace cxl {

void CXLEffects::Update(int tempo) {
	const int freq = d_lowpassFreq;
	const int q = d_lowpassQ;
	if (freq == 127) {
		d_filter.SetBypass(true); }
	else {
		d_filter.SetBypass(false);
		d_filter.SetCutoff(pow(freq/127.0f, 2.0f));
		// scale Q-max slightly under 1.0
		d_filter.SetQ(sqrt(q/128.9f)); }

	d_filter.Update(tempo);

	d_delay.SetTime(d_delayTime);
	d_delay.SetFeedbackGain(d_delayFeedback / 127.0);
	d_delay.Update(tempo);

	d_reducer.d_midi = d_reduce;
	d_reducer.Update(tempo); }


void CXLEffects::Process(float* inputs, float* outputs) {
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
