#pragma once
#include "src/ral/raldsp/raldsp_filter.hxx"
#include "src/ral/raldsp/raldsp_multimode.hxx"
#include "src/ral/raldsp/raldsp_ratereducer.hxx"
#include "src/ral/raldsp/raldsp_syncdelay.hxx"

namespace rqdq {
namespace cxl {

class CXLEffects {
public:
	CXLEffects();
	void Update(int);
	void Process(float*, float*);

	void Initialize() {
		d_lowpassFreq = 127;
		d_lowpassQ = 0;

		d_delaySend = 0;
		d_delayTime = 16;
		d_delayFeedback = 0;

		d_reduce = 0;

		d_eqGain = 0;
		d_eqCenter = 64;
		d_eqGain2 = 0;
		d_eqCenter2 = 64; }

public:
	int d_lowpassFreq{127};
	int d_lowpassQ{0};

	int d_delaySend{0};      // 0-127 = 0...1.0
	int d_delayTime{16};     // 0-127, 128th notes
	int d_delayFeedback{0};  // 0-127 = 0...1.0

	int d_reduce{0};

	int d_eqGain{0};         // -64-63
	int d_eqCenter{64};      // 0-127
	int d_eqGain2{-1};        // 0-127
	int d_eqCenter2{-1};     // 0-127

private:
	raldsp::MultiModeFilter d_eq;
	raldsp::CXLFilter d_filter{1};
	raldsp::SyncDelay d_delay{1};
	raldsp::RateReducer d_reducer{1}; };


}  // namespace cxl
}  // namespace rqdq
