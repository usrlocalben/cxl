#include "sampler.hxx"

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

namespace cxl {

void SingleSampler::Trigger(int note, double volume, int ppqstamp) {
	auto& wave = waveTable_.Get(params_.waveId);
	if (!wave.d_loaded) {
		state_ = Idle{}; }
	else {
		Playing s;
		// https://forum.juce.com/t/mapping-frequencies-to-midi-notes/1762/2
		s.delta = pow(2.0, (note+params_.tuningInNotes-kSamplerRootNote)/12.0 + (params_.tuningInCents / 1200.0));
		s.delta *= wave.d_freq / 44100.0;  // XXX
		s.wavePtr = &wave;
		wave.AddReference();

		auto t = adSamples(params_.attackTime);
		s.gainVelocity = t > 0 ? (volume / t) : volume;

		s.targetGain = volume;
		s.curGain = 0.0;
		s.position = wave.d_selectionBegin;
		state_ = s; }}


void SingleSampler::Stop() {
	if (!std::holds_alternative<Idle>(state_)) {
		state_ = Idle{}; }}


void SingleSampler::Update(int tempo) {}


void SingleSampler::Process(float* inputs, float* outputs) {
	auto& out = outputs[0];
	out = 0;

	if (std::holds_alternative<Playing>(state_)) {
		auto& s = std::get<Playing>(state_);
		s.curGain += s.gainVelocity;
		if (s.gainVelocity > 0) {
			// attack
			if (s.curGain >= s.targetGain) {
				// switch to decay
				s.targetGain = 0;
				auto dv = (params_.decayTime == 127) ? 0 : s.curGain / adSamples(params_.decayTime);
				s.gainVelocity = -dv; }}
		else {
			// decay
			if (s.curGain <= s.targetGain) {
				state_ = Idle{}; }}}

	if (std::holds_alternative<Idle>(state_)) {
		return; }

	auto& s = std::get<Playing>(state_);
	const auto[sampleLeft, sampleRight] = s.wavePtr->Sample(s.position);
	out = (sampleLeft + sampleRight) / 2;
	out *= s.curGain;
	s.position += s.delta;

	if (params_.loopType == LoopType::Loop) {
		if (int(s.position) > s.wavePtr->d_loopEnd) {
			s.position -= s.wavePtr->d_loopEnd - s.wavePtr->d_loopBegin; }}
	else {
		if (int(s.position) >= s.wavePtr->d_selectionEnd) {
			state_ = Idle{}; }}}


}  // namespace cxl
}  // namespace rqdq
