#include "src/cxl/cxl_controller.hxx"

#include "src/cxl/cxl_reactor.hxx"
#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/cxl_unit_view.hxx"
#include "src/rcl/rclw/rclw_console.hxx"

#include <array>
#include <deque>
#include <sstream>
#include <string>
#include <vector>

#define NOMINMAX
#include <Windows.h>

namespace rqdq {
namespace cxl {

using ScanCode = rclw::ScanCode;

CXLUnitController::CXLUnitController(
	rclw::Console& console,
	CXLUnit& unit)
	:d_console(console), d_unit(unit) {
	d_downKeys.resize(256, false); }


void CXLUnitController::OnCXLUnitChanged() {
	CXLUnitView(d_unit, d_selectedTrack, d_selectedPage, d_keyHistory).Draw(d_console); }


void CXLUnitController::OnConsoleInputAvailable() {
	// process stdin
	INPUT_RECORD record;
	DWORD numRead;
	if (ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &record, 1, &numRead) == 0) {
		throw std::runtime_error("ReadConsoleInput failure"); }
	if (record.EventType == KEY_EVENT) {
		const auto& e = record.Event.KeyEvent;

		{std::stringstream ss;
		ss << (e.bKeyDown != 0?'D':'U');
		ss << " " << std::hex << e.dwControlKeyState << std::dec;
		ss << " " << e.uChar.AsciiChar;
		ss << " " << e.wRepeatCount;
		ss << " " << e.wVirtualKeyCode;
		ss << " " << e.wVirtualScanCode << "  ";
		d_keyHistory.emplace_back(ss.str());
		if (d_keyHistory.size() > 8) {
			d_keyHistory.pop_front(); }}

		if (e.wVirtualScanCode<256) {
			d_downKeys[e.wVirtualScanCode] = (e.bKeyDown != 0); }

		if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && e.wVirtualScanCode==ScanCode::Q) {
			Reactor::GetInstance().Stop(); }
		else if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && (ScanCode::Key1<=e.wVirtualScanCode && e.wVirtualScanCode<=ScanCode::Key8)) {
			// Ctrl+1...Ctrl+8
			d_selectedTrack = e.wVirtualScanCode - ScanCode::Key1;
			CXLUnitView(d_unit, d_selectedTrack, d_selectedPage, d_keyHistory).Draw(d_console); }
		else if ((e.bKeyDown != 0) && e.dwControlKeyState==0) {
			if (e.wVirtualScanCode == ScanCode::Semicolon) { d_unit.Stop(); }
			else if (e.wVirtualScanCode == ScanCode::Quote) { d_unit.Play(); }
			else if (e.wVirtualScanCode == ScanCode::Comma || e.wVirtualScanCode == ScanCode::Period) {

				int amt = (e.wVirtualScanCode == ScanCode::Comma ? -1 : 1);
				if ((e.dwControlKeyState & rclw::kCKShift) != 0u) {
					// XXX msft internal bug 9311951
					// https://github.com/Microsoft/WSL/issues/1188
					amt *= 10; }

				if      (d_downKeys[ScanCode::T]) { d_unit.Adjust(d_selectedTrack, d_selectedPage*8+0, amt); }
				else if (d_downKeys[ScanCode::Y]) { d_unit.Adjust(d_selectedTrack, d_selectedPage*8+1, amt); }
				else if (d_downKeys[ScanCode::U]) { d_unit.Adjust(d_selectedTrack, d_selectedPage*8+2, amt); }
				else if (d_downKeys[ScanCode::I]) { d_unit.Adjust(d_selectedTrack, d_selectedPage*8+3, amt); }
				else if (d_downKeys[ScanCode::G]) { d_unit.Adjust(d_selectedTrack, d_selectedPage*8+4, amt); }
				else if (d_downKeys[ScanCode::H]) { d_unit.Adjust(d_selectedTrack, d_selectedPage*8+5, amt); }
				else if (d_downKeys[ScanCode::J]) { d_unit.Adjust(d_selectedTrack, d_selectedPage*8+6, amt); }
				else if (d_downKeys[ScanCode::K]) { d_unit.Adjust(d_selectedTrack, d_selectedPage*8+7, amt); }
				else if (d_downKeys[ScanCode::Equals]) {
					d_unit.SetTempo(d_unit.GetTempo() + amt); }}
			else {
				const std::array<int, 16> gridscan = { 2, 3, 4, 5, 16, 17, 18, 19, 30, 31, 32, 33, 44, 45, 46, 47 };
				int i;
				for (i=0; i<16; i++) {
					if (e.wVirtualScanCode == gridscan[i]) {
						break; }}
				if (i<16) {
					d_unit.ToggleTrackGridNote(d_selectedTrack, i); }}}}}


}  // namespace cxl
}  // namespace rqdq
