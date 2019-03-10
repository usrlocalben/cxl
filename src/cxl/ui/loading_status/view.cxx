#include "src/cxl/ui/loading_status/view.hxx"

#include <stdexcept>

#include "src/cxl/unit.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

LoadingStatus::LoadingStatus(CXLUnit& unit) :d_unit(unit) {
	d_unit.d_loaderStateChanged.connect(this, &LoadingStatus::onLoaderStateChange); }


void LoadingStatus::onLoaderStateChange() {
	d_dirty = true; }


std::pair<int, int> LoadingStatus::Pack(int w, int h) {
	return {60, 4}; }


int LoadingStatus::GetType() {
	return TextKit::WT_FIXED; }


bool LoadingStatus::HandleKeyEvent(TextKit::KeyEvent e) {
	return false; }


const rcls::TextCanvas& LoadingStatus::Draw(int width, int height) {
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
