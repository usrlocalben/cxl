#pragma once
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"

namespace rqdq {
namespace raldsp {

class Distortor : public IAudioDevice {
public:
	Distortor(int numChannels)
		:d_numChannels(numChannels) {}

	// IAudioDevice
	void Update(int) override;
	void Process(float*, float*) override;

public:
	int d_threshold = 127;

private:
	const int d_numChannels; };


}  // namespace raldsp
}  // namespace rqdq
