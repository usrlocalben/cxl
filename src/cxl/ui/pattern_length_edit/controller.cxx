#include "controller.hxx"

#include <algorithm>

#include "src/cxl/ui/pattern_length_edit/view.hxx"
#include "src/rcl/rcls/rcls_console.hxx"
#include "src/textkit/keyevent.hxx"

namespace rqdq {
namespace cxl {

using ScanCode = rcls::ScanCode;

PatternLengthEditController::PatternLengthEditController(int value)
	:value_(value), view_{MakePatternLengthEditView(value_)} {}


bool PatternLengthEditController::HandleKeyEvent(TextKit::KeyEvent e) {
	if (e.down && e.control==0) {
		if (e.scanCode == ScanCode::Comma || e.scanCode == ScanCode::Period) {
			int offset = (e.scanCode == ScanCode::Comma ? -16 : 16);
			int newValue = std::clamp(value_+offset, 16, 64);
			if (newValue != value_) {
				value_ = newValue; }
			return true; }
		if (e.scanCode == ScanCode::Enter) {
			if (onSuccess) {
				onSuccess(value_);}
			return true; }
		if (e.scanCode == ScanCode::Esc) {
			if (onCancel) {
				onCancel(); }
			return true;}}
	return false; }


}  // namespace cxl
}  // namespace rqdq
