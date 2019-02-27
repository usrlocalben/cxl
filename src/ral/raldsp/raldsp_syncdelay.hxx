#pragma once
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"

#include <vector>

namespace rqdq {
namespace raldsp {

class SyncDelay : public IAudioDevice {
public:
	SyncDelay(int numChannels);

	// IAudioDevice
	void Update(int) override;
	void Process(float*, float*) override;

	void SetTime(int t) { d_delayTime = t; }
	void SetFeedbackGain(float amt) { d_feedbackGain = amt; }
	int GetNumChannels();

private:
	void AdvanceHead();

private:
	// user params
	int d_delayTime = 16;  // length of delay in 128th notes
	float d_feedbackGain = 0;

	// updated per tick
	int d_delayTimeInSamples;
	bool d_valid = false;

	// updated per samples
	int d_head = 0;

	std::vector<std::vector<float>> d_bufs; };

}  // namespace raldsp
}  // namespace rqdq
