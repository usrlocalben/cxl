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

namespace {

using SC = rclw::ScanCode;

/**
 * LUT for converting a 4x4 matrix of scan-codes to
 * indices for grid positions, track#, pattern# etc
 */
constexpr std::array<char, 16> kGridScanLUT = {
	SC::Key1, SC::Key2, SC::Key3, SC::Key4,
	SC::Q,    SC::W,    SC::E,    SC::R,
	SC::A,    SC::S,    SC::D,    SC::F,
	SC::Z,    SC::X,    SC::C,    SC::V };

/**
 * LUT for converting a 4x4 matrix of scan-codes to
 * indices for parameter controls
 */
constexpr std::array<char, 8> kParamScanLUT = {
	SC::T, SC::Y, SC::U, SC::I,
	SC::G, SC::H, SC::J, SC::K, };

}  // namespace

namespace cxl {

using ScanCode = rclw::ScanCode;

CXLUnitController::CXLUnitController(
	rclw::Console& console,
	CXLUnit& unit)
	:d_console(console), d_unit(unit) {
	d_downKeys.resize(256, false); }


void CXLUnitController::OnCXLUnitChanged() {
	CXLUnitView(d_unit, d_selectedTrack, d_selectedPage, d_keyHistory, d_enableKeyDebug).Draw(d_console); }


void CXLUnitController::OnConsoleInputAvailable() {
	// process stdin
	INPUT_RECORD record;
	DWORD numRead;
	if (ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &record, 1, &numRead) == 0) {
		throw std::runtime_error("ReadConsoleInput failure"); }
	if (record.EventType == KEY_EVENT) {
		auto& e = record.Event.KeyEvent;

		AddKeyDebuggerEvent(&e);

		if (e.wVirtualScanCode<256) {
			d_downKeys[e.wVirtualScanCode] = (e.bKeyDown != 0); }

		if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && e.wVirtualScanCode==ScanCode::Q) {
			Reactor::GetInstance().Stop(); }
		else if ((e.bKeyDown != 0) && e.dwControlKeyState==rclw::kCKLeftCtrl && (ScanCode::Key1<=e.wVirtualScanCode && e.wVirtualScanCode<=ScanCode::Key8)) {
			// Ctrl+1...Ctrl+8
			d_selectedTrack = e.wVirtualScanCode - ScanCode::Key1;
			CXLUnitView(d_unit, d_selectedTrack, d_selectedPage, d_keyHistory, d_enableKeyDebug).Draw(d_console); }
		else if ((e.bKeyDown != 0) && e.dwControlKeyState==0) {
			if (e.wVirtualScanCode == ScanCode::Semicolon) { d_unit.Stop(); }
			else if (e.wVirtualScanCode == ScanCode::Quote) { d_unit.Play(); }
			else if (e.wVirtualScanCode == ScanCode::F5) { d_unit.SaveKit(); }
			else if (e.wVirtualScanCode == ScanCode::F6) { d_unit.LoadKit(); }
			else if (e.wVirtualScanCode == ScanCode::F7) { d_unit.DecrementKit(); }
			else if (e.wVirtualScanCode == ScanCode::F8) { d_unit.IncrementKit(); }
			else if (e.wVirtualScanCode == ScanCode::Comma || e.wVirtualScanCode == ScanCode::Period) {

				int amt = (e.wVirtualScanCode == ScanCode::Comma ? -1 : 1);
				if ((e.dwControlKeyState & rclw::kCKShift) != 0u) {
					// XXX does not work because of MSFT internal bug 9311951
					// https://github.com/Microsoft/WSL/issues/1188
					amt *= 10; }

				auto it = std::find_if(begin(kParamScanLUT), end(kParamScanLUT),
									   [&](auto& item) { return d_downKeys[item]; });
				if (it != end(kParamScanLUT)) {
					int idx = std::distance(begin(kParamScanLUT), it);
					d_unit.Adjust(d_selectedTrack, d_selectedPage*8+idx, amt); }
				else if (d_downKeys[ScanCode::Equals]) {
					d_unit.SetTempo(std::max(10, d_unit.GetTempo() + amt)); }}
			else {
				auto it = std::find_if(begin(kGridScanLUT), end(kGridScanLUT),
									   [&](auto &item) { return e.wVirtualScanCode == item; });
				if (it != end(kGridScanLUT)) {
					const int idx = std::distance(begin(kGridScanLUT), it);
					d_unit.ToggleTrackGridNote(d_selectedTrack, idx); }}}}}


void CXLUnitController::AddKeyDebuggerEvent(void *data) {
	auto& e = *reinterpret_cast<KEY_EVENT_RECORD*>(data);
	if (d_enableKeyDebug) {
		std::stringstream ss;
		ss << (e.bKeyDown != 0?'D':'U');
		ss << " " << std::hex << e.dwControlKeyState << std::dec;
		ss << " " << e.uChar.AsciiChar;
		ss << " " << e.wRepeatCount;
		ss << " " << e.wVirtualKeyCode;
		ss << " " << e.wVirtualScanCode << "  ";
		d_keyHistory.emplace_back(ss.str());
		if (d_keyHistory.size() > 8) {
			d_keyHistory.pop_front(); }}}


}  // namespace cxl
}  // namespace rqdq
