#include "src/ral/raldsp/raldsp_biquad.hxx"

#include <cmath>

namespace rqdq {
namespace raldsp {

void BiQuad::Configure(BiQuad::Type type, float dbGain, float freq, float q, float sampleRate) {
	const auto A = pow(10, dbGain/40);
	const auto omega = 2 * kPi * freq / sampleRate;
	const auto sn = sin(omega);
	const auto cs = cos(omega);
	// const auto alpha = sn * sinh(kLN2 /2 * bw * omega /sn);
	const auto alpha = sn / (2 * q);
	const auto beta = -1; // sqrt(A + A);

	float a0, a1, a2;
	float b0, b1, b2;

	switch (type) {
	case Type::LPF:
		b0 = (1 - cs) / 2;
		b1 = 1 - cs;
		b2 = (1 - cs) / 2;
		a0 = 1 + alpha;
		a1 = -2 * cs;
		a2 = 1 - alpha;
		break;
	case Type::HPF:
		b0 = (1 + cs) / 2;
		b1 = -(1 + cs);
		b2 = (1 + cs) / 2;
		a0 = 1 + alpha;
		a1 = -2 * cs;
		a2 = 1 - alpha;
		break;
	case Type::BPF:
		b0 = alpha;
		b1 = 0;
		b2 = -alpha;
		a0 = 1 + alpha;
		a1 = -2 * cs;
		a2 = 1 - alpha;
		break;
	case Type::NOTCH:
		b0 = 1;
		b1 = -2 * cs;
		b2 = 1;
		a0 = 1 + alpha;
		a1 = -2 * cs;
		a2 = 1 - alpha;
		break;
	case Type::PEQ:
		b0 = 1 + (alpha * A);
		b1 = -2 * cs;
		b2 = 1 - (alpha * A);
		a0 = 1 + (alpha / A);
		a1 = -2 * cs;
		a2 = 1 - (alpha / A);
		break;
	case Type::LSH:
		b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
		b1 = 2 * A * ((A - 1) - (A + 1) * cs);
		b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
		a0 = (A + 1) + (A - 1) * cs + beta * sn;
		a1 = -2 * ((A - 1) + (A + 1) * cs);
		a2 = (A + 1) + (A - 1) * cs - beta * sn;
		break;
	case Type::HSH:
		b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
		b1 = -2 * A * ((A - 1) + (A + 1) * cs);
		b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
		a0 = (A + 1) - (A - 1) * cs + beta * sn;
		a1 = 2 * ((A - 1) - (A + 1) * cs);
		a2 = (A + 1) - (A - 1) * cs - beta * sn;
		break; }

	b0_ = b0 / a0;
	b1_ = b1 / a0;
	b2_ = b2 / a0;
	a1_ = a1 / a0;
	a2_ = a2 / a0; }


}  // namespace raldsp
}  // namespace rqdq
