#include "src/cxl/ui/alert/view.hxx"

#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

#include <string>

#include "3rdparty/fmt/include/fmt/printf.h"

namespace rqdq {
namespace cxl {

using ScanCode = rclw::ScanCode;

Alert::Alert(std::string text) :d_text(std::move(text)) {}


std::pair<int, int> Alert::Pack(int w, int h) {
	return {d_text.length() + 2, 1 }; }


int Alert::GetType() {
	return TextKit::WT_FIXED; }


bool Alert::HandleKeyEvent(TextKit::KeyEvent e) {
	return false; }


const rclw::ConsoleCanvas& Alert::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = d_canvas;
	if (d_dirty) {
		d_dirty = false;
		out.Resize(width, height);
		out.Clear();
		auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::StrongBrown);
		Fill(out, lo);
		WriteXY(out, 1, 0, d_text);}

	return out; }


}  // namespace cxl
}  // namespace rqdq
