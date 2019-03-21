#include "src/ral/raldsp/raldsp_ratereducer.hxx"

#include <algorithm>
#include <cassert>
#include <vector>

namespace rqdq {
namespace raldsp {

RateReducer::RateReducer(int numChannels) {
	d_bufs.resize(numChannels);
	for (auto& buf : d_bufs) {
		std::fill(begin(buf), end(buf), 0); }
	d_emit.resize(numChannels); }


void RateReducer::Update(int tempo) {}


void RateReducer::Process(float* inputs, float *outputs) {
	const int windowSize = d_midi + 1;
	assert(1<=windowSize && windowSize <= 128);
	
	for (int i=0; i<GetNumChannels(); i++) {
		d_bufs[i][d_idx] = inputs[i]; }

	d_idx++;
	if (d_idx >= windowSize) {
		for (int ci=0; ci<GetNumChannels(); ci++) {
			float ax = 0;
			for (int si=0; si<windowSize; si++) {
				ax += d_bufs[ci][si]; }
			d_emit[ci] = ax / windowSize; }
		d_idx = 0; }

	for (int i=0; i<GetNumChannels(); i++) {
		outputs[i] = d_emit[i]; }}

}  // namespace raldsp
}  // namespace rqdq
