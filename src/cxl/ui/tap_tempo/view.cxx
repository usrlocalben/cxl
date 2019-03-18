#include "src/cxl/ui/tap_tempo/view.hxx"

#include <string>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

TapTempoView::TapTempoView(const int& taps) :d_taps(taps) {}


std::pair<int, int> TapTempoView::Pack(int w, int h) {
	return { 14, 3 }; }


int TapTempoView::GetType() {
	return TextKit::WT_FIXED; }


bool TapTempoView::HandleKeyEvent(TextKit::KeyEvent e) {
	return false; }


const rcls::TextCanvas& TapTempoView::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = d_canvas;
	if (d_taps != d_lastTaps) {
		d_lastTaps = d_taps;
		out.Resize(width, height);
		out.Clear();
		auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBrown);
		Fill(out, lo);
		WriteXY(out, 2, 0, "tap tempo");
		// "  tap tempo   "
		// "  x  _  _  _  "
		WriteXY(out, 2, 2, d_taps >= 1 ? "x" : ".");
		WriteXY(out, 5, 2, d_taps >= 2 ? "x" : ".");
		WriteXY(out, 8, 2, d_taps >= 3 ? "x" : ".");
		WriteXY(out,11, 2, d_taps >= 4 ? "x" : ".");}
	return out; }


}  // namespace cxl
}  // namespace rqdq
