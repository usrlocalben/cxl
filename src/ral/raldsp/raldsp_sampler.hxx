#pragma once
#include "src/ral/raldsp/raldsp_filter.hxx"
#include "src/ral/raldsp/raldsp_idspoutput.hxx"
#include "src/ral/raldsp/raldsp_syncdelay.hxx"
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
		case Loop: e = Oneshot; break; }}
	void Decrement() {
		switch (e) {
		case Loop: e = None; break;
		case Oneshot: e = Loop; break; }}
	void offset(const int dir) {
		assert(dir==1 || dir==-1);
		if (dir == 1) {
			Increment(); }
		else {
			Decrement(); }}
private:
	Enumerator e; };


class VoiceState {
public:
	enum Enumerator {
		 Attack = 0
		,Decay = 1
		,Unknown };
	VoiceState() :e(Unknown) {}
	VoiceState(Enumerator val) :e(val){}
	operator Enumerator() const { assert(e!=Unknown); return e; }
private:
	Enumerator e; };


struct VoiceParameters {
	int waveId = 0;
	int tuning = 0;         // pitch fine-tune in cents
	int cutoff = 127;       // 0-127, 127=filter off
	int resonance = 0;      // 0-127
	LoopType loopType = LoopType::None;
	int attackPct = 0;      // [0...100] % of sample duration
	int decayPct = 90;      // [0...100] % of sample duration
	DecayMode decayMode = DecayMode::End;
	int gain = 10;          // [-600...+60] db gain*10
	int pan = 0;            // [-63...+63] 0=center

	int delaySend = 0;      // 0-127 = 0...1.0
	int delayTime = 16;     // 0-127, 128th notes
	int delayFeedback = 0;};// 0-127 = 0...1.0


class PlayState {
public:
	ralw::MPCWave *wavePtr;

	bool active = false;

	double position;
	double delta;

	double curGain;
	double targetGain;
	VoiceState state;
	double attackVelocity;
	double decayVelocity;
	int decayBegin;

	CXLFilter filter; };


class SingleSampler : public IDSPOutput {
public:
	// IDSPOutput
	void Update(int) override;
	void Process(float*, float*) override;

public:
	SingleSampler(ralw::WaveTable& wt)
		:d_waveTable(wt) {}

	void Trigger(int note, double velocity, int ppqstamp);
	void Stop();

public:
	PlayState d_state;
	ralw::WaveTable& d_waveTable;
	VoiceParameters d_params;
	SyncDelay d_delay; };


}  // close package namespace
}  // close enterprise namespace
