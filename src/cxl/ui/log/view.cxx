#include "view.hxx"

#include "src/cxl/log.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"

namespace rqdq {
namespace cxl {

LogView::LogView() = default;


std::pair<int, int> LogView::Pack(int w, int h) {
	return {w, h}; }


int LogView::GetType() {
	return TextKit::WT_BOX; }


const rcls::TextCanvas& LogView::Draw(int width, int height) {
	auto& log = Log::GetInstance();
	auto& out = canvas_;
	out.Resize(width, height);
	out.Clear();

	Fill(out, rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack));
	int head = log.GetHeadIdx();
	for (int y=0; y<height; y++) {
		WriteXY(out, 0, y, log.GetEntry(height-y-1, head)); }

	return out; }


}  // namespace cxl
}  // namespace rqdq
