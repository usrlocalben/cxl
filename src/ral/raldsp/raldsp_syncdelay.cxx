#include "src/ral/raldsp/raldsp_syncdelay.hxx"
#include "src/ral/raldsp/raldsp_idspoutput.hxx"

#include <algorithm>
#include <utility>
#include <vector>

namespace rqdq {

namespace {

constexpr int kMaxDelayInSamples = 44100*10;
constexpr int kSampleRate = 44100;

}  // namespace

namespace raldsp {

SyncDelay::SyncDelay() {
	d_bufLeft.resize(kMaxDelayInSamples, 0.0f);
	d_bufRight.resize(kMaxDelayInSamples, 0.0f); }


void SyncDelay::Update(int tempo) {
	const double delayTimeInNotes  = d_delayTime / 127.0;
	const double tempoInNotesPerMinute = tempo / 10.0 / 4.0;
	const double samplesPerMinute = (kSampleRate * 60.0);
	d_delayTimeInSamples = samplesPerMinute / tempoInNotesPerMinute * delayTimeInNotes;
	d_valid = true; };


void SyncDelay::Process(float* inputs, float* outputs) {
	auto& inLeft = inputs[0];
	auto& inRight = inputs[1];
	auto& outLeft = outputs[0];
	auto& outRight = outputs[1];
	if (!d_valid) {
		outLeft = 0, outRight = 0;
		return; }
	int tail = (d_head + d_delayTimeInSamples) % kMaxDelayInSamples;
	outLeft = d_bufLeft[d_head];
	outRight = d_bufRight[d_head];
	d_bufLeft[tail] = inLeft + outLeft*d_feedbackGain;
	d_bufRight[tail] = inRight + outRight*d_feedbackGain;
	AdvanceHead(); }


void SyncDelay::AdvanceHead() {
	d_head++;
	if (d_head >= kMaxDelayInSamples) {
		d_head = 0; }}

}  // namespace raldsp
}  // namespace rqdq
