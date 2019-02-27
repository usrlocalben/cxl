#pragma once
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"

#include <cassert>
#include <utility>

namespace rqdq {
namespace raldsp {


class DecayMode {
public:
	enum Enumerator {
		 Begin = 0
		,End = 1
		,Unknown };
	DecayMode() :e(Unknown){}
	DecayMode(Enumerator val) :e(val){}
	operator Enumerator() const { assert(e!=Unknown); return e; }
private:
	Enumerator e; };


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
public:
	// IAudioDevice
	void Update(int) override;
	void Process(float*, float*) override;

public:
	SingleSampler(ralw::WaveTable& wt)
		:d_waveTable(wt) {}

	void Trigger(int note, double velocity, int ppqstamp);
	void Stop();

	void Initialize() {
		d_waveId = 0;
		d_tuning = 0;
		d_loopType = LoopType::None;
		d_attackPct = 0;
		d_decayPct = 90;
		d_decayMode = DecayMode::End;
		d_gain = 10; }

public:
	// parameters
	int d_waveId = 0;
	int d_tuning = 0;         // pitch fine-tune in cents
	LoopType d_loopType = LoopType::None;
	int d_attackPct = 0;      // [0...100] % of sample duration
	int d_decayPct = 90;      // [0...100] % of sample duration
	DecayMode d_decayMode = DecayMode::End;
	int d_gain = 10;          // [-600...+60] db gain*10

private:
	// playing state
	ralw::MPCWave *d_wavePtr;
	bool d_isActive = false;
	double d_position;
	double d_delta;
	double d_curGain;
	double d_targetGain;
	EnvelopeState d_envState;
	double d_attackVelocity;
	double d_decayVelocity;
	int d_decayBegin;

	ralw::WaveTable& d_waveTable; };


}  // close package namespace
}  // close enterprise namespace
