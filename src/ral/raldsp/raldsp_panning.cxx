#include "src/ral/raldsp/raldsp_panning.hxx"

#include <cmath>

namespace rqdq {
namespace raldsp {

PanningLUT::PanningLUT() {
	for (int i=0; i<128; i++) {
		leftGain[i]  = sqrt((128-i)/128.0);
		rightGain[i] = sqrt(     i /128.0); }}

}  // namespace raldsp
}  // namespace rqdq
