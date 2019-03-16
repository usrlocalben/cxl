#include "src/cxl/ui/loading/view.hxx"

#include <stdexcept>
#include <utility>

#include "src/cxl/unit.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

LoadingView::LoadingView(const CXLUnit& unit) :d_unit(unit) {}


void LoadingView::Invalidate() {
	d_dirty = true; }


std::pair<int, int> LoadingView::Pack(int w, int h) {
	return {60, 4}; }


int LoadingView::GetType() {
	return TextKit::WT_FIXED; }


const rcls::TextCanvas& LoadingView::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = d_canvas;
	if (d_dirty) {
		d_dirty = false;
		out.Resize(width, height);
		out.Clear();
		auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::White);
		Fill(out, lo);
		WriteXY(out, 0, 0, fmt::sprintf("Loading...", width, height));
		WriteXY(out, 0, 1, d_unit.GetLoadingName());
		DrawPercentageBar(out, 0, 2, 60, d_unit.GetLoadingProgress()); }

	return out; }


}  // namespace cxl
}  // namespace rqdq
