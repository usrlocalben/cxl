#pragma once
#include <cassert>
#include <utility>
#include <variant>

#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"

namespace rqdq {
namespace raldsp {


class LoopType {
public:
	enum Enumerator {
		 None = 0
		,Loop = 1
		,Oneshot = 2
		,Unknown
	};
	LoopType() :e(Unknown){}
	LoopType(Enumerator val) :e(val){}
	operator Enumerator() const { assert(e!=Unknown); return e; }
	void Increment() {
		switch (e) {
		case None: e = Loop; break;
		case Loop: e = Oneshot; break;
		default: {}}}
	void Decrement() {
		switch (e) {
		case Loop: e = None; break;
		case Oneshot: e = Loop; break;
		default: {} }}
	void offset(const int dir) {
		assert(dir==1 || dir==-1);
		if (dir == 1) {
			Increment(); }
		else {
			Decrement(); }}
private:
	Enumerator e; };


class EnvelopeState {
public:
	enum Enumerator {
		 Attack = 0
		,Decay = 1
		,Unknown };
	EnvelopeState() :e(Unknown) {}
	EnvelopeState(Enumerator val) :e(val){}
	operator Enumerator() const { assert(e!=Unknown); return e; }
private:
	Enumerator e; };


class SingleSampler : public IAudioDevice {

	struct Parameters {
		int waveId{0};
		int tuningInNotes{0};  // [-64...63] pitch semitone offset
		int tuningInCents{0};  // pitch fine-tune in cents
		LoopType loopType{LoopType::None};
		int attackTime{0};     // [0...127] 0...4sec, pow^2 curve
		int decayTime{127};    // [0...127] 0...4sec, pow^2 curve, 127=inf
		int gain{10}; };       // [-600...+60] db gain*10

	// std::variant FSM
	struct Idle {};
	struct Playing {
		ralw::MPCWave* wavePtr;
		double position;
		double delta;
		double curGain;
		double targetGain;
		double gainVelocity;
		~Playing() {
			if (wavePtr != nullptr) {
				wavePtr->Release(); }} };

	using State = std::variant<Idle, Playing>;

public:
	SingleSampler(ralw::WaveTable& wt) :waveTable_(wt) {}

public:
	// IAudioDevice
	void Update(int) override;
	void Process(float*, float*) override;

	// user interface
	void Initialize() {
		params_ = Parameters{}; }

	// audio thread only
	void Trigger(int note, double velocity, int ppqstamp);
	void Stop();

public:
	Parameters params_{};
private:
	State state_{Idle{}};
	ralw::WaveTable& waveTable_; };


}  // close package namespace
}  // close enterprise namespace
