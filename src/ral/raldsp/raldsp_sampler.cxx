#include "src/ral/raldsp/raldsp_sampler.hxx"

#include <utility>
#include <variant>

namespace rqdq {
namespace {

constexpr int kMidiC3 = 48;

constexpr int kSamplerRootNote = kMidiC3;

constexpr double kMaxAttackTimeInMillis{4000.0};

/**
 * compute attack/decay time in #samples.
 * 127 = kMaxAttackTimeInMillis
 * input is pow^2'd to shift precision to the lower end of the range
 */
int adSamples(int midiValue) {
	auto s = midiValue / 127.0;
	auto millis = s*s * kMaxAttackTimeInMillis;
	auto samples = millis * 44100.0 / 1000.0;
	return samples; }


}  // namespace

namespace raldsp {

void SingleSampler::Trigger(int note, double volume, int ppqstamp) {
	auto& wave = waveTable.Get(params.waveId);
	if (!wave.d_loaded) {
		state = Idle{}; }
	else {
		Playing s;
		// https://forum.juce.com/t/mapping-frequencies-to-midi-notes/1762/2
		s.delta = pow(2.0, (note+params.tuningInNotes-kSamplerRootNote)/12.0 + (params.tuningInCents / 1200.0));
		s.delta *= wave.d_freq / 44100.0;  // XXX
		s.wavePtr = &wave;
		wave.AddReference();

		auto t = adSamples(params.attackTime);
		s.gainVelocity = t > 0 ? (volume / t) : volume;

		s.targetGain = volume;
		s.curGain = 0.0;
		s.position = wave.d_selectionBegin;
		state = s; }}


void SingleSampler::Stop() {
	if (!std::holds_alternative<Idle>(state)) {
		state = Idle{}; }}


void SingleSampler::Update(int tempo) {}


void SingleSampler::Process(float* inputs, float* outputs) {
	auto& out = outputs[0];
	out = 0;

	if (std::holds_alternative<Playing>(state)) {
		auto& s = std::get<Playing>(state);
		s.curGain += s.gainVelocity;
		if (s.gainVelocity > 0) {
			// attack
			if (s.curGain >= s.targetGain) {
				// switch to decay
				s.targetGain = 0;
				auto dv = (params.decayTime == 127) ? 0 : s.curGain / adSamples(params.decayTime);
				s.gainVelocity = -dv; }}
		else {
			// decay
			if (s.curGain <= s.targetGain) {
				state = Idle{}; }}}

	if (std::holds_alternative<Idle>(state)) {
		return; }

	auto& s = std::get<Playing>(state);
	const auto[sampleLeft, sampleRight] = s.wavePtr->Sample(s.position);
	out = (sampleLeft + sampleRight) / 2;
	out *= s.curGain;
	s.position += s.delta;

	if (params.loopType == LoopType::Loop) {
		if (int(s.position) > s.wavePtr->d_loopEnd) {
			s.position -= s.wavePtr->d_loopEnd - s.wavePtr->d_loopBegin; }}
	else {
		if (int(s.position) >= s.wavePtr->d_selectionEnd) {
			state = Idle{}; }}}


}  // namespace raldsp
}  // namespace rqdq
