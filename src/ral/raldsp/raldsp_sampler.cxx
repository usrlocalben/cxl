#include "src/ral/raldsp/raldsp_sampler.hxx"

#include <array>
#include <utility>

namespace rqdq {
namespace raldsp {


void SingleSampler::Trigger(int note, double velocity, int ppqstamp) {
	auto& wave = d_waveTable.Get(d_waveId);
	if (!wave.d_loaded) {
		return; }

	if (d_isActive) {
		d_wavePtr->Release();
		d_isActive = false; }

	// https://forum.juce.com/t/mapping-frequencies-to-midi-notes/1762/2
	d_delta = pow(2.0, d_tuning / 1200.0);
	d_delta *= wave.d_freq / 44100.0;  // XXX
	d_wavePtr = &wave;
	d_isActive = true;
	wave.AddReference();

	const int samplesPerCent = (wave.d_selectionEnd - wave.d_selectionBegin) / 100;
	if (d_attackPct != 0) {
		d_attackVelocity = velocity / (d_attackPct*samplesPerCent); }
	else {
		d_attackVelocity = velocity; }

	const int invDecayPct = 100 - d_decayPct;
	if (invDecayPct != 0) {
		d_decayVelocity = velocity / (invDecayPct*samplesPerCent); }
	else {
		d_decayVelocity = velocity; }

	d_decayBegin = wave.d_selectionEnd - invDecayPct*samplesPerCent;

	d_targetGain = velocity;
	d_curGain = 0.0;
	d_position = wave.d_selectionBegin;
	d_envState = EnvelopeState::Attack; }



void SingleSampler::Stop() {
	d_isActive = false;
	d_wavePtr->Release(); }


void SingleSampler::Update(int tempo) {}


void SingleSampler::Process(float* inputs, float* outputs) {
	auto& out = outputs[0];

	out = 0;
	if (d_isActive) {
		if (d_envState == EnvelopeState::Attack) {
			d_curGain += d_attackVelocity;
			if (d_curGain >= d_targetGain) {
				d_curGain = d_targetGain;
				if (d_decayMode == DecayMode::Begin) {
					d_envState = EnvelopeState::Decay; }}}

		if (d_envState == EnvelopeState::Decay) {
			d_curGain -= d_decayVelocity;
			if (d_curGain < 0) {
				d_curGain = 0; }}

		if (d_loopType != LoopType::Loop) {
			if ((d_position >= d_decayBegin) &&
				(d_envState != EnvelopeState::Decay)) {
				d_envState = EnvelopeState::Decay; }}

		const auto[sampleLeft, sampleRight] = d_wavePtr->Sample(d_position);
		out = (sampleLeft + sampleRight) / 2;
		out *= d_curGain;
		d_position += d_delta;

		if (d_curGain >= d_targetGain) {
			if (d_decayMode == DecayMode::Begin) {
				d_envState = EnvelopeState::Decay; }}

		if (d_curGain <= 0) {
			d_isActive = false;
			d_wavePtr->Release(); }
		else {
			if (d_loopType == LoopType::Loop) {
				if (int(d_position) > d_wavePtr->d_loopEnd) {
					d_position -= d_wavePtr->d_loopEnd - d_wavePtr->d_loopBegin; }}
			else {
				if (int(d_position) >= d_wavePtr->d_selectionEnd) {
					d_isActive = false;
					d_wavePtr->Release(); }}}}}


}  // namespace raldsp
}  // namespace rqdq
