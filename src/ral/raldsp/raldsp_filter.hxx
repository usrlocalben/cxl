#pragma once
#include "src/ral/raldsp/raldsp_iaudiodevice.hxx"

#include <utility>
#include <vector>

namespace rqdq {
namespace raldsp {

class CXLFilter : public IAudioDevice {
public:
	CXLFilter(int numChannels) {
		d_buf0.resize(numChannels, 0);
		d_buf1.resize(numChannels, 0); }

	// IAudioDevice
	void Update(int tempo) override {}
	void Process(float* inputs, float* outputs) override {
		for (int i=0; i<GetNumChannels(); i++) {
			d_buf0[i] = d_buf0[i] + d_f * (inputs[i] - d_buf0[i] + d_fb * (d_buf0[i] - d_buf1[i]));
			d_buf1[i] = d_buf1[i] + d_f *                                 (d_buf0[i] - d_buf1[i]);
			outputs[i] = d_bypass ? inputs[i] : d_buf1[i]; }}

public:
	void SetCutoff(double value) {
		d_f = value;
		UpdateInternal();}

	void SetQ(double value) {
		d_q = value;
		UpdateInternal();}

	void SetBypass(bool value) {
		if (d_bypass && !value) {
			Reset(); }
		d_bypass = value; }

	int GetNumChannels() const {
		return d_buf0.size(); }

	void Reset() {
		for (int i=0; i<GetNumChannels(); i++) {
			d_buf0[i] = 0.0;
			d_buf1[i] = 0.0; }}

private:
	void UpdateInternal() {
		d_fb = d_q + d_q / (1.0 - d_f); }

private:
	bool d_bypass;
	std::vector<double> d_buf0;
	std::vector<double> d_buf1;
	double d_fb;
	double d_f;
	double d_q; };


}  // close package namespace
}  // close enterprise namespace
