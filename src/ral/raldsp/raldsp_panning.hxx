#pragma once
#include <array>
#include <cassert>

namespace rqdq {
namespace raldsp {

class PanningLUT {
public:
	PanningLUT();

	void Pan(const float* inputs, float* outputs, int setting) {
		assert(-64 <= setting && setting <= 63);
		outputs[0] = inputs[0] * leftGain[setting+64];
		outputs[1] = inputs[0] * rightGain[setting+64]; }

private:
	std::array<float, 128> leftGain;
	std::array<float, 128> rightGain; };


}  // namespace raldsp
}  // namespace rqdq
