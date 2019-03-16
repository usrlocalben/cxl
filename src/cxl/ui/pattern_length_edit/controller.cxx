#include "src/cxl/ui/pattern_length_edit/controller.hxx"

#include <algorithm>

#include "src/rcl/rcls/rcls_console.hxx"
#include "src/textkit/keyevent.hxx"

namespace rqdq {
namespace cxl {

using ScanCode = rcls::ScanCode;

PatternLengthEditController::PatternLengthEditController(int value)
	:d_value(value), d_view{d_value} {}


bool PatternLengthEditController::HandleKeyEvent(TextKit::KeyEvent e) {
	if (e.down && e.control==0) {
		if (e.scanCode == ScanCode::Comma || e.scanCode == ScanCode::Period) {
			int offset = (e.scanCode == ScanCode::Comma ? -16 : 16);
			int newValue = std::clamp(d_value+offset, 16, 64);
			if (newValue != d_value) {
				d_value = newValue;
				d_view.Invalidate(); }
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


}  // namespace cxl
}  // namespace rqdq
