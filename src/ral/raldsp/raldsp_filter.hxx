#pragma once
#include "src/ral/raldsp/raldsp_idspoutput.hxx"

#include <utility>

namespace rqdq {
namespace raldsp {

class CXLFilter : public IDSPOutput {
public:
	// IDSPOutput
	void Update(int tempo) override {}
	void Process(float* inputs, float* outputs) override {
		auto& il = inputs[0];
		auto& ir = inputs[1];
		if (d_bypass) {
			outputs[0] = il, outputs[1] = ir; }
		else {
			d_l0 = d_l0 + d_f * (il - d_l0 + d_fb * (d_l0 - d_l1));
			d_l1 = d_l1 + d_f *                     (d_l0 - d_l1) ;

			d_r0 = d_r0 + d_f * (ir - d_r0 + d_fb * (d_r0 - d_r1));
			d_r1 = d_r1 + d_f *                     (d_r0 - d_r1) ;
			outputs[0] = d_l1, outputs[1] = d_r1; }}

public:
	void SetCutoff(double value) {
		d_f = value;
		UpdateInternal();}

	void SetResonance(double value) {
		d_q = value;
		UpdateInternal();}

	void SetBypass(bool value) {
		if (d_bypass && !value) {
			Reset(); }
		d_bypass = value; }

	void Reset() {
		d_r0 = 0.0;
		d_r1 = 0.0;
		d_l0 = 0.0;
		d_l1 = 0.0; }

private:
	void UpdateInternal() {
		d_fb = d_q + d_q / (1.0 - d_f); }

private:
	bool d_bypass;
	double d_r0, d_r1;
	double d_l0, d_l1;
	double d_fb;
	double d_f;
	double d_q; };


}  // close package namespace
}  // close enterprise namespace
