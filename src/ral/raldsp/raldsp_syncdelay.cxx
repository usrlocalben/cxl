#include "src/ral/raldsp/raldsp_syncdelay.hxx"

#include <algorithm>
#include <utility>
#include <vector>

namespace rqdq {
namespace {

constexpr int kMaxDelayInSamples = 44100*10;
constexpr int kSampleRate = 44100;

}  // namespace

namespace raldsp {

SyncDelay::SyncDelay(int numChannels) {
	d_bufs.resize(numChannels);
	for (auto& buf : d_bufs) {
		buf.resize(kMaxDelayInSamples, 0.0f); }}


void SyncDelay::Update(int tempo) {
	const double delayTimeInNotes  = d_delayTime / 127.0;
	const double tempoInNotesPerMinute = tempo / 10.0 / 4.0;
	const double samplesPerMinute = (kSampleRate * 60.0);
	d_delayTimeInSamples = samplesPerMinute / tempoInNotesPerMinute * delayTimeInNotes;
	d_valid = true; };


int SyncDelay::GetNumChannels() {
	return d_bufs.size(); }


void SyncDelay::Process(float* inputs, float* outputs) {
	int tail = (d_head + d_delayTimeInSamples) % kMaxDelayInSamples;
	for (int i = 0; i < GetNumChannels(); i++) {
		auto& buf = d_bufs[i];
		auto& input = inputs[i];
		auto& output = outputs[i];
		output = buf[d_head];
		buf[tail] = input + output*d_feedbackGain; }
	AdvanceHead(); }


void SyncDelay::AdvanceHead() {
	d_head++;
	if (d_head >= kMaxDelayInSamples) {
		d_head = 0; }}


}  // namespace raldsp
}  // namespace rqdq
