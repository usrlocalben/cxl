#include "src/cxl/ui/cxl_ui_alert.hxx"

#include "src/cxl/cxl_widget.hxx"
#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"

#include <string>

#include "3rdparty/fmt/include/fmt/printf.h"

namespace rqdq {
namespace cxl {

using ScanCode = rclw::ScanCode;

Alert::Alert(const std::string& text) :d_text(text) {}


std::pair<int, int> Alert::Pack(int w, int h) {
	return {d_text.length() + 2, 1 }; }


int Alert::GetType() {
	return WT_FIXED; }


bool Alert::HandleKeyEvent(KEY_EVENT_RECORD e) {
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
