/**
 * biquad audio equalizer filters
 * described here:
 * https://www.w3.org/2011/audio/audio-eq-cookbook.html
 * https://github.com/shepazu/Audio-EQ-Cookbook
 * https://github.com/wooters/miniDSP/blob/master/biquad.c
 * https://sourceforge.net/p/equalizerapo/code/HEAD/tree/trunk/filters/BiQuad.h
 */
#pragma once
namespace rqdq {
namespace raldsp {

constexpr float kPi{3.14159265358979323846};

constexpr float kLN2{0.693147180559945309417};


class BiQuad {
	enum class Type {
		LPF,
		HPF,
		BPF,
		NOTCH,
		PEQ,
		LSH,
		HSH, };
public:
	BiQuad(Type type, float dbGain, float freq, float q, float sampleRate=44100.0f) {
		Configure(type, dbGain, freq, q, sampleRate); }

	void Configure(Type type, float dbGain, float freq, float q, float sampleRate);

	float Process(float s) {
		const auto result = b0_*s + b1_*x1_ + b2_*x2_ \
		                          - a1_*y1_ - a2_*y2_;
		x2_ = x1_; x1_ = s;
		y2_ = y1_; y1_ = result;
		return result; }

private:
	float b0_, b1_, b2_, a1_, a2_;
	float x1_{0}, x2_{0};
	float y1_{0}, y2_{0}; };


}  // namespace raldsp
}  // namespace rqdq
