#include "view.hxx"

#include <string>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

Alert::Alert(std::string text) :text_(std::move(text)) {}


std::pair<int, int> Alert::Pack(int w, int h) {
	return {text_.length() + 2, 1 }; }


int Alert::GetType() {
	return TextKit::WT_FIXED; }


bool Alert::HandleKeyEvent(TextKit::KeyEvent e) {
	return false; }


bool Alert::Refresh() {
	if (first_) {
		first_ = false;
		return true; }
	return false; }


const rcls::TextCanvas& Alert::Draw(int width, int height) {
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
		WriteXY(out, 1, 0, text_);}

	return out; }


}  // namespace cxl
}  // namespace rqdq
