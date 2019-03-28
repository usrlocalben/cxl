#include "view.hxx"

#include <stdexcept>
#include <utility>

#include "src/cxl/unit.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

LoadingView::LoadingView(const CXLUnit& unit) :unit_(unit) {}


std::pair<int, int> LoadingView::Pack(int w, int h) {
	return {60, 4}; }


int LoadingView::GetType() {
	return TextKit::WT_FIXED; }


bool LoadingView::Refresh() {
	bool updated = false;
	float pct = unit_.GetLoadingProgress();
	if (pct != pct_) {
		updated = true;
		pct_ = pct; }
	return updated; }


const rcls::TextCanvas& LoadingView::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = canvas_;
	bool updated = Refresh();
	if (first_ || updated) {
		first_ = false;
		out.Resize(width, height);
		out.Clear();
		auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::White);
		Fill(out, lo);
		WriteXY(out, 0, 0, fmt::sprintf("Loading...", width, height));
		WriteXY(out, 0, 1, unit_.GetLoadingName());
		DrawPercentageBar(out, 0, 2, 60, pct_); }
	return out; }


}  // namespace cxl
}  // namespace rqdq
