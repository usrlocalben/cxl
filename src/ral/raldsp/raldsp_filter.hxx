#pragma once
#include <utility>

namespace rqdq {
namespace raldsp {


class CXLFilter {
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

	std::pair<float, float> Process(float il, float ir) {
		if (d_bypass) {
			return {il, ir}; }
		else {
			d_l0 = d_l0 + d_f * (il - d_l0 + d_fb * (d_l0 - d_l1));
			d_l1 = d_l1 + d_f *                     (d_l0 - d_l1) ;

			d_r0 = d_r0 + d_f * (il - d_r0 + d_fb * (d_r0 - d_r1));
			d_r1 = d_r1 + d_f *                     (d_r0 - d_r1) ;
			return {d_l1, d_r1}; }}

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
