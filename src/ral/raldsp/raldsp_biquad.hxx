#pragma once

constexpr float kPi{3.14159265358979323846};
constexpr float kLN2{0.693147180559945309417};

class PeakingEQ {
public:
	BiQuad(float dbGain, float freq, float bw, float rate=44100.0f) {
		const auto A = pow(10, dbGain/40);
		const auto omega = 2 * kPi * freq / rate;
		const auto sn = sin(omega);
		const auto cs = cos(omega);
		const auto alpha = sn * sinh(kLN2 /2 * bw * omega /sn);
		const auto beta = sqrt(A + A);

		// peaking-EQ
		float b0 = 1 + (alpha * A);
		float b1 = -2 * cs;
		float b2 = 1 - (alpha * A);
		float a0 = 1 + (alpha / A);
		float a1 = -2 * cs;
		float a2 = 1 - (alpha / A);

		b0_ = b0 / a0;
		b1_ = b1 / a0;
		b2_ = b2 / a0;
		a1_ = a1 / a0;
		a2_ = a2 / a0;

		x1_ = x2_ = 0;
		y1_ = y2_ = 0; }

	float Process(float s) {
		const auto result = b0_*s + b1_*x1_ + b2_*x2_ - a1_*y1_ - a2_*y2_;
		x2_ = x1_; x1_ = s;
		y2_ = y1_; y1_ = result;
		return result; }

	static PeakingEQ MakePeakingEQ(int gain_, int freq_, int width_) {
		float gain = std::clamp(gain_, -64, 63) / 63.0 * 12;
		float freq = std::clamp(freq_, 0, 127) / 127.0 * 20000;
		float width = std::clamp(width_, 0, 127) / 127.0 * 2.0;
		return BiQuad(gain, freq, width); }

private:
	float b0, b1, b2, a1, a2; };
