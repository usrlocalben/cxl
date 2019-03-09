#pragma once
#include "src/ral/raldsp/raldsp_distortion.hxx"

namespace rqdq {
namespace cxl {

struct CXLChannelStrip {
	void Update(int tempo);
	void Process(float* inputs, float* outputs);
	void Initialize() {
		d_gain = 100;
		d_mute = false;
		d_pan = 0;
		d_distortion = 0;
		d_send1 = 0;
		d_send2 = 0; }
	int d_gain = 100;
	bool d_mute = false;
	int d_pan =0;
	int d_distortion = 0;
	int d_send1 = 0;
	int d_send2 = 0;

public:
	/* void SetGain(int value) { d_gain = value; }
	void SetPan(int value) { d_pan = value; }
	void SetMute(bool value) { d_mute = value; }
	int GetGain() { return d_gain; }
	int GetPan() { return d_pan; }
	bool GetMute() { return d_mute; } */

	raldsp::Distortor d_distortor{1}; };


}  // namespace cxl
}  // namespace rqdq
