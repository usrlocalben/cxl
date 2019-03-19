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


std::pair<int, int> LoadingView::Pack(int w, int h) {
	return {60, 4}; }


int LoadingView::GetType() {
	return TextKit::WT_FIXED; }


bool LoadingView::Refresh() {
	bool updated = false;
	float pct = d_unit.GetLoadingProgress();
	if (pct != d_pct) {
		updated = true;
		d_pct = pct; }
	return updated; }


const rcls::TextCanvas& LoadingView::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = d_canvas;
	bool updated = Refresh();
	if (d_first || updated) {
		d_first = false;
		out.Resize(width, height);
		out.Clear();
		auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::White);
		Fill(out, lo);
		WriteXY(out, 0, 0, fmt::sprintf("Loading...", width, height));
		WriteXY(out, 0, 1, d_unit.GetLoadingName());
		DrawPercentageBar(out, 0, 2, 60, d_pct); }
	return out; }


}  // namespace cxl
}  // namespace rqdq
