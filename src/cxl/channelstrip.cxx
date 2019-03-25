#include "channelstrip.hxx"

#include "src/ral/raldsp/raldsp_panning.hxx"

namespace rqdq {
namespace {

raldsp::PanningLUT panningLUT;

}  // namespace

namespace cxl {

void CXLChannelStrip::Update(int tempo) {}


void CXLChannelStrip::Process(float* inputs, float* outputs) {
	float& in = inputs[0];
	float tmp = 0;

	d_distortor.d_threshold = 127 - d_distortion;
	d_distortor.Process(inputs, &tmp);
	inputs[0] = tmp;

	in *= (d_gain / float(100.0));
	panningLUT.Pan(&in, outputs, d_pan);
	outputs[2] = outputs[0] * (d_send1/127.0);
	outputs[3] = outputs[1] * (d_send1/127.0);
	outputs[4] = outputs[0] * (d_send2/127.0);
	outputs[5] = outputs[1] * (d_send2/127.0); }


}  // namespace cxl
}  // namespace rqdq
