#include "src/ral/raldsp/raldsp_multimode.hxx"

#include <cmath>

namespace rqdq {
namespace {

constexpr float kPi{3.14159265358979323846};

constexpr float kLN2{0.693147180559945309417};


}  // namespace

namespace raldsp {

void MultiModeFilter::Configure(MultiModeFilter::Mode type,
                                float gainIndB,
                                float centerFreqInHz,
                                float param,
                                ParamType paramType,
                                float sampleFreqInHz) {
	const auto A = pow(10, gainIndB/40);
	const auto omega = 2 * kPi * centerFreqInHz / sampleFreqInHz;
	const auto sn = sin(omega);
	const auto cs = cos(omega);

	float alpha;
	switch (paramType) {
	case ParamType::Q:  alpha = sn / (2 * param);                     break;
	case ParamType::BW: alpha = sn * sinh(kLN2/2 * param * omega/sn); break;
	case ParamType::S:  alpha = sn/2 * sqrt((A+1/A)*(1/param-1)+2);   break; }

	float a0, a1, a2;
	float b0, b1, b2;

	switch (type) {
	case Mode::LP:
		b0 = (1 - cs) / 2;
		b1 = 1 - cs;
		b2 = (1 - cs) / 2;
		a0 = 1 + alpha;
		a1 = -2 * cs;
		a2 = 1 - alpha;
		break;
	case Mode::HP:
		b0 = (1 + cs) / 2;
		b1 = -(1 + cs);
		b2 = (1 + cs) / 2;
		a0 = 1 + alpha;
		a1 = -2 * cs;
		a2 = 1 - alpha;
		break;
	case Mode::BP:
		b0 = alpha;
		b1 = 0;
		b2 = -alpha;
		a0 = 1 + alpha;
		a1 = -2 * cs;
		a2 = 1 - alpha;
		break;
	case Mode::NOTCH:
		b0 = 1;
		b1 = -2 * cs;
		b2 = 1;
		a0 = 1 + alpha;
		a1 = -2 * cs;
		a2 = 1 - alpha;
		break;
	case Mode::PEQ:
		b0 = 1 + (alpha * A);
		b1 = -2 * cs;
		b2 = 1 - (alpha * A);
		a0 = 1 + (alpha / A);
		a1 = -2 * cs;
		a2 = 1 - (alpha / A);
		break;
	case Mode::LSH:
		b0 = A * ((A + 1) - (A - 1) * cs + 2*sqrt(A)*alpha);
		b1 = 2 * A * ((A - 1) - (A + 1) * cs);
		b2 = A * ((A + 1) - (A - 1) * cs - 2*sqrt(A)*alpha);
		a0 = (A + 1) + (A - 1) * cs + 2*sqrt(A)*alpha;
		a1 = -2 * ((A - 1) + (A + 1) * cs);
		a2 = (A + 1) + (A - 1) * cs - 2*sqrt(A)*alpha;
		break;
	case Mode::HSH:
		b0 = A * ((A + 1) + (A - 1) * cs + 2*sqrt(A)*alpha);
		b1 = -2 * A * ((A - 1) + (A + 1) * cs);
		b2 = A * ((A + 1) + (A - 1) * cs - 2*sqrt(A)*alpha);
		a0 = (A + 1) - (A - 1) * cs + 2*sqrt(A)*alpha;
		a1 = 2 * ((A - 1) - (A + 1) * cs);
		a2 = (A + 1) - (A - 1) * cs - 2*sqrt(A)*alpha;
		break; }

	b0_ = b0 / a0;
	b1_ = b1 / a0;
	b2_ = b2 / a0;
	a1_ = a1 / a0;
	a2_ = a2 / a0; }


}  // namespace raldsp
}  // namespace rqdq
