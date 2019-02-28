#include "src/ral/raldsp/raldsp_distortion.hxx"

#include <cmath>

namespace rqdq {
namespace raldsp {


void Distortor::Update(int tempo) {}


/**
 * https://www.musicdsp.org/en/latest/Effects/203-fold-back-distortion.html
 */
void Distortor::Process(float* inputs, float* outputs) {
    using std::fabs, std::fmod;
	const float T = d_threshold / 127.0;
    const float makeupGain = 1.0 / T;

	for (int i=0; i<d_numChannels; i++) {
		float signal = inputs[i];
        signal = fabs(fabs(fmod(signal - T, T*4)) - T*2) - T;
		outputs[i] = signal * makeupGain; }}


}  // namespace raldsp
}  // namespace rqdq
