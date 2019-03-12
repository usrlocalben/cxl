#include "src/cxl/ui/log/view.hxx"

#include "src/cxl/log.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {

LogView::LogView(TextKit::MainLoop& loop) :d_loop(loop) {
	auto& log = Log::GetInstance();
	log.d_updated.connect([&]() { onLogWrite(); }); }


std::pair<int, int> LogView::Pack(int w, int h) {
	return {w, h}; }


int LogView::GetType() {
	return TextKit::WT_BOX; }


const rcls::TextCanvas& LogView::Draw(int width, int height) {
	auto& log = Log::GetInstance();
	auto& out = d_canvas;
	out.Resize(width, height);
	out.Clear();

	Fill(out, rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack));
	int head = log.GetHeadIdx();
	for (int y=0; y<height; y++) {
		WriteXY(out, 0, y, log.GetEntry(height-y-1, head)); }

	return out; }


bool LogView::HandleKeyEvent(const TextKit::KeyEvent e) {
	return false; }


void LogView::onLogWrite() {
	d_loop.DrawScreenEventually(); }


}  // namespace cxl
}  // namespace rqdq
