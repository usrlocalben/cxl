#include "view.hxx"

#include <stdexcept>
#include <string>
#include <utility>

#include "src/rcl/rcls/rcls_text_canvas.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

PatternLengthEditView::PatternLengthEditView(const int& value)
	:valueSrc_(value) {}


std::pair<int, int> PatternLengthEditView::Pack(int w, int h) {
	return {std::string("Pattern Length").length(), 2};}


int PatternLengthEditView::GetType() {
	return TextKit::WT_FIXED; }


bool PatternLengthEditView::Refresh() {
	bool updated = false;
	if (valueSrc_ != value_) {
		updated = true;
		value_ = valueSrc_; }
	return updated; }


const rcls::TextCanvas& PatternLengthEditView::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = canvas_;
	bool updated = Refresh();
	if (updated) {
		out.Resize(width, height);
		out.Clear();
		auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::White);
		Fill(out, lo);
		WriteXY(out, 0, 0, "Pattern Length");
		WriteXY(out, 6, 1, fmt::sprintf("%d", value_)); }
	return out; }


}  // namespace cxl
}  // namespace rqdq
