#include "src/cxl/ui/host/view.hxx"

#include "src/cxl/host.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

HostView::HostView(CXLASIOHost& host) :d_host(host) {}


std::pair<int, int> HostView::Pack(int w, int h) {
	return {w, h}; }


int HostView::GetType() {
	return TextKit::WT_BOX; }


const rcls::TextCanvas& HostView::Draw(int width, int height) {
	auto& out = d_canvas;
	out.Resize(width, height);
	out.Clear();

	Fill(out, rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack));

	auto driverName = d_host.GetRunningDriverName();
	auto sampleRate = d_host.GetRunningSampleRate();
	auto bufferSize = d_host.GetRunningBufferSize();

	int xpos = 4;
	int y = 4;

	WriteXY(out, xpos, y, fmt::sprintf("ASIO Driver: %s", driverName)); y++;
	y++;
	WriteXY(out, xpos, y, fmt::sprintf("Sample Rate: %d", sampleRate)); y++;
	WriteXY(out, xpos, y, fmt::sprintf("Buffer Size: %d", bufferSize)); y++;
	// WriteXY(out, xpos, y, fmt::sprintf("supportsOutputReadyOptimization: %s", info.supportsOutputReadyOptimization ? "Yes" : "No")); y++;

	return out; }


}  // namespace cxl
}  // namespace rqdq
