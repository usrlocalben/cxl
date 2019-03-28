#include "view.hxx"

#include <string>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

TapTempoView::TapTempoView(const int& taps) :tapsSrc_(taps) {}


std::pair<int, int> TapTempoView::Pack(int w, int h) {
	return { 14, 3 }; }


int TapTempoView::GetType() {
	return TextKit::WT_FIXED; }


bool TapTempoView::HandleKeyEvent(TextKit::KeyEvent e) {
	return false; }


bool TapTempoView::Refresh() {
	bool updated = false;
	if (tapsSrc_ != taps_) {
		updated = true;
		taps_ = tapsSrc_; }
	return updated; }


const rcls::TextCanvas& TapTempoView::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = canvas_;
	bool updated = Refresh();
	if (updated) {
		out.Resize(width, height);
		out.Clear();
		auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBrown);
		Fill(out, lo);
		WriteXY(out, 2, 0, "tap tempo");
		// "  tap tempo   "
		// "  x  _  _  _  "
		WriteXY(out, 2, 2, taps_ >= 1 ? "x" : ".");
		WriteXY(out, 5, 2, taps_ >= 2 ? "x" : ".");
		WriteXY(out, 8, 2, taps_ >= 3 ? "x" : ".");
		WriteXY(out,11, 2, taps_ >= 4 ? "x" : ".");}
	return out; }


}  // namespace cxl
}  // namespace rqdq
