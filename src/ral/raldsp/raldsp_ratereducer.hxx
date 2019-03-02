#pragma once
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"

#include <array>
#include <vector>

namespace rqdq {
namespace raldsp {

class RateReducer : public IAudioDevice {
public:
	RateReducer(int numChannels);
	int GetNumChannels() const {
		return d_bufs.size(); }

	// IAudioDevice
	void Update(int) override;
	void Process(float*, float*) override;

	std::vector<std::array<float, 128>> d_bufs;
	std::vector<float> d_emit;
	int d_idx=0;
	int d_midi = 0; };


}  // namespace raldsp
}  // namespace rqdq
