#include "src/cxl/ui/pattern_length_edit/view.hxx"

#include "src/rcl/rclw/rclw_console.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

#include <string>

#include "3rdparty/fmt/include/fmt/printf.h"

namespace rqdq {
namespace cxl {

using ScanCode = rclw::ScanCode;

PatternLengthEdit::PatternLengthEdit(int value) :d_value(value) {}


std::pair<int, int> PatternLengthEdit::Pack(int w, int h) {
	return {std::string("Pattern Length").length(), 2};}


int PatternLengthEdit::GetType() {
	return TextKit::WT_FIXED; }


bool PatternLengthEdit::HandleKeyEvent(TextKit::KeyEvent e) {
	if (e.down && e.control==0) {
		if (e.scanCode == ScanCode::Comma || e.scanCode == ScanCode::Period) {

			int offset = (e.scanCode == ScanCode::Comma ? -16 : 16);
			int newValue = std::clamp(d_value+offset, 16, 64);
			if (newValue != d_value) {
				d_value = newValue;
				d_dirty = true; }
			return true; }
		if (e.scanCode == ScanCode::Enter) {
			if (onSuccess) {
				onSuccess(d_value);}
			return true; }
		if (e.scanCode == ScanCode::Esc) {
			if (onCancel) {
				onCancel(); }
			return true;}}
	return false; }


const rclw::ConsoleCanvas& PatternLengthEdit::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = d_canvas;
	if (d_dirty) {
		d_dirty = false;
		out.Resize(width, height);
		out.Clear();
		auto lo = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::White);
		Fill(out, lo);
		WriteXY(out, 0, 0, "Pattern Length");
		WriteXY(out, 6, 1, fmt::sprintf("%d", d_value)); }

	return out; }


}  // namespace cxl
}  // namespace rqdq
