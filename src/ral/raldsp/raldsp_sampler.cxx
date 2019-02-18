#include "src/ral/raldsp/raldsp_sampler.hxx"
#include "src/ral/raldsp/raldsp_filter.hxx"

#include <array>
#include <utility>

namespace rqdq {

namespace {

class PanningLUT {
public:
	PanningLUT() {
		for (int i=0; i<101; i++) {
			left[i] = sqrt( (101-i)/101.0 );
			right[i] = sqrt(      i /101.0 );
		}}
private:
	std::array<double, 101> left;
	std::array<double, 101> right;
};

PanningLUT panningLUT;

}  // close unnamed namespace

namespace raldsp {


void SingleSampler::Trigger(int note, double velocity, int ppqstamp) {
	auto& wave = d_waveTable.Get(d_params.waveId);
	if (!wave.d_loaded) {
		return; }

	if (d_state.active) {
		d_state.wavePtr->Release();
		d_state.active = false; }

	// https://forum.juce.com/t/mapping-frequencies-to-midi-notes/1762/2
	d_state.delta = pow(2.0, d_params.tuning / 1200.0);
	d_state.delta *= wave.d_freq / 44100.0;  // XXX
	d_state.wavePtr = &wave;
	d_state.active = true;
	wave.AddReference();

	const int samplesPerCent = (wave.d_selectionEnd - wave.d_selectionBegin) / 100;
	if (d_params.attackPct != 0) {
		d_state.attackVelocity = velocity / (d_params.attackPct*samplesPerCent); }
	else {
		d_state.attackVelocity = velocity; }

	const int invDecayPct = 100 - d_params.decayPct;
	if (invDecayPct != 0) {
		d_state.decayVelocity = velocity / (invDecayPct*samplesPerCent); }
	else {
		d_state.decayVelocity = velocity; }

	d_state.decayBegin = wave.d_selectionEnd - invDecayPct*samplesPerCent;

	d_state.targetGain = velocity;
	d_state.curGain = 0.0;
	d_state.position = wave.d_selectionBegin;
	d_state.state = VoiceState::Attack;

	const int cutoff = d_params.cutoff;
	const int resonance = d_params.resonance;
	if (cutoff<127 || resonance>0) {
		d_state.filter.SetBypass(false);
		d_state.filter.SetCutoff(1.0 - sqrt((127-std::clamp(cutoff, 1, 126)) / 127.0));
		d_state.filter.SetResonance(1.0 - sqrt((127-std::clamp(resonance, 1, 126)) / 127.0));
		d_state.filter.Reset(); }
	else {
		d_state.filter.SetBypass(true); }}


void SingleSampler::Stop() {
	d_state.active = false;
	d_state.wavePtr->Release(); }


std::pair<float, float> SingleSampler::GetNextSample() {
	if (!d_state.active) {
		return {0.0f, 0.0f}; }

	float sl, sr;

	std::tie(sl, sr) = d_state.wavePtr->Sample(d_state.position);
	std::tie(sl, sr) = d_state.filter.Process(sl, sr);

	if (d_state.state == VoiceState::Attack) {
		d_state.curGain += d_state.attackVelocity;
		if (d_state.curGain >= d_state.targetGain) {
			d_state.curGain = d_state.targetGain;
			if (d_params.decayMode == DecayMode::Begin) {
				d_state.state = VoiceState::Decay; }}}

	if (d_state.state == VoiceState::Decay) {
		d_state.curGain -= d_state.decayVelocity;
		if (d_state.curGain < 0) {
			d_state.curGain = 0; }}

	if (d_params.loopType != LoopType::Loop) {
		if ((d_state.position >= d_state.decayBegin) &&
			(d_state.state != VoiceState::Decay)) {
			d_state.state = VoiceState::Decay; }}

	sl *= d_state.curGain;
	sr *= d_state.curGain;

	// XXX internal panning would be applied here

	d_state.position += d_state.delta;

	if (d_state.curGain >= d_state.targetGain) {
		if (d_params.decayMode == DecayMode::Begin) {
			d_state.state = VoiceState::Decay; }}

	if (d_state.curGain <= 0) {
		d_state.active = false;
		d_state.wavePtr->Release(); }
	else {
		if (d_params.loopType == LoopType::Loop) {
			if (int(d_state.position) > d_state.wavePtr->d_loopEnd) {
				d_state.position -= d_state.wavePtr->d_loopEnd - d_state.wavePtr->d_loopBegin; }}
		else {
			if (int(d_state.position) >= d_state.wavePtr->d_selectionEnd) {
				d_state.active = false;
				d_state.wavePtr->Release(); }}}

	return {sl, sr}; }


}  // namespace raldsp
}  // namespace rqdq
