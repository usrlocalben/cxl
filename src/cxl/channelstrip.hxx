#pragma once
#include "src/ral/raldsp/raldsp_distortion.hxx"

namespace rqdq {
namespace cxl {

class CXLChannelStrip {
	struct Parameters {
		int gain{100};
		bool mute{false};
		int pan{0};
		int distortion{0};
		int send1{0};
		int send2{0}; };

public:
	void Update(int tempo);
	void Process(float* inputs, float* outputs);
	void Initialize() {
		params_ = Parameters{}; }

	Parameters params_{};

private:
	raldsp::Distortor distortor_{1}; };


}  // namespace cxl
}  // namespace rqdq
