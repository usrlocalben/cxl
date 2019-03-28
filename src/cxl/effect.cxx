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
	const int freq = params_.lowpassFreq;
	const int q = params_.lowpassQ;
	if (freq == 127) {
		filter_.SetBypass(true); }
	else {
		filter_.SetBypass(false);
		filter_.SetCutoff(pow(freq/127.0F, 2.0F));
		// scale Q-max slightly under 1.0
		filter_.SetQ(sqrt(q/128.9F)); }

	filter_.Update(tempo);

	delay_.SetTime(params_.delayTime);
	delay_.SetFeedbackGain(params_.delayFeedback / 127.0);
	delay_.Update(tempo);

	reducer_.d_midi = params_.reduce;
	reducer_.Update(tempo);

	if (params_.eqGain != curEQGain_ || params_.eqCenter != curEQCenter_) {
		curEQGain_ = params_.eqGain;
		curEQCenter_ = params_.eqCenter;
		float gain = std::clamp(params_.eqGain, -64, 63) / 63.0 * 12;
		float freq = pow(std::clamp(params_.eqCenter, 0, 127) / 127.0, 2.0) * 8000.0;
		eq_.Configure(raldsp::MultiModeFilter::Mode::PEQ,
		               gain,
		               freq,
		               kEQQ,
		               raldsp::MultiModeFilter::ParamType::Q,
		               44100.0); }}


void CXLEffects::Process(float* inputs, float* outputs) {
	inputs[0] = eq_.Process(inputs[0]);
	float filtered;
	filter_.Process(inputs, &filtered);

	float delaySend = params_.delaySend / 127.0;
	float toDelay = filtered * delaySend;
	float fromDelay;
	delay_.Process(&toDelay, &fromDelay);

	float filteredPlusDelay = filtered + fromDelay;
	reducer_.Process(&filteredPlusDelay, outputs); }


}  // namespace cxl
}  // namespace rqdq
