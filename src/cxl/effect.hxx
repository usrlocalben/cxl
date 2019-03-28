#pragma once
#include "src/ral/raldsp/raldsp_filter.hxx"
#include "src/ral/raldsp/raldsp_multimode.hxx"
#include "src/ral/raldsp/raldsp_ratereducer.hxx"
#include "src/ral/raldsp/raldsp_syncdelay.hxx"

namespace rqdq {
namespace cxl {

class CXLEffects {
	struct Parameters {
		int lowpassFreq{127};
		int lowpassQ{0};

		int delaySend{0};      // 0-127 = 0...1.0
		int delayTime{16};     // 0-127, 128th notes
		int delayFeedback{0};  // 0-127 = 0...1.0

		int reduce{0};

		int eqGain{0};         // -64-63
		int eqCenter{64}; };    // 0-127

public:
	CXLEffects();
	void Update(int /*tempo*/);
	void Process(float* /*inputs*/, float* /*outputs*/);

	void Initialize() {
		params_ = Parameters{}; }

public:
	Parameters params_{};

private:
	raldsp::MultiModeFilter eq_;
	raldsp::CXLFilter filter_{1};
	raldsp::SyncDelay delay_{1};
	raldsp::RateReducer reducer_{1};
	int curEQGain_{-1};
	int curEQCenter_{-1}; };


}  // namespace cxl
}  // namespace rqdq
