#include "src/cxl/ui/pattern_length_edit/view.hxx"

#include <stdexcept>
#include <string>
#include <utility>

#include "src/rcl/rcls/rcls_text_canvas.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

PatternLengthEditView::PatternLengthEditView(const int& value)
	:d_valueSrc(value) {}


std::pair<int, int> PatternLengthEditView::Pack(int w, int h) {
	return {std::string("Pattern Length").length(), 2};}


int PatternLengthEditView::GetType() {
	return TextKit::WT_FIXED; }


bool PatternLengthEditView::Refresh() {
	bool updated = false;
	if (d_valueSrc != d_value) {
		updated = true;
		d_value = d_valueSrc; }
	return updated; }


const rcls::TextCanvas& PatternLengthEditView::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = d_canvas;
	bool updated = Refresh();
	if (updated) {
		out.Resize(width, height);
		out.Clear();
		auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::White);
		Fill(out, lo);
		WriteXY(out, 0, 0, "Pattern Length");
		WriteXY(out, 6, 1, fmt::sprintf("%d", d_value)); }
	return out; }


}  // namespace cxl
}  // namespace rqdq
