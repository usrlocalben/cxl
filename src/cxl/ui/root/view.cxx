#include "src/cxl/ui/root/view.hxx"

#include <string>
#include <utility>

#include "src/cxl/ui/host/view.hxx"
#include "src/cxl/ui/log/view.hxx"
#include "src/cxl/ui/pattern/view.hxx"
#include "src/cxl/ui/root/state.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

RootView::RootView(const CXLUnit& unit,
                   HostView& hostView,
				   PatternView& patternView,
				   LogView& logView,
				   const int& mode
				   )
	:d_unit(unit),
	d_hostView(hostView),
	d_patternView(patternView),
	d_logView(logView),
	d_mode(mode) {}


std::pair<int, int> RootView::Pack(int w, int h) {
	return {80, 25}; }


int RootView::GetType() {
	return TextKit::WT_BOX; }


const rcls::TextCanvas& RootView::Draw(int width, int height) {
	auto& out = d_canvas;
	out.Resize(width, height);
	out.Clear();
	WriteXY(out, 0, 0, DrawHeader(width));

	//if (d_enableKeyDebug) {
	//	WriteXY(out, width-20, height-15, DrawKeyHistory()); }

	WriteXY(out, 0, height-1, DrawTransportIndicator(width));

	if (d_mode == UM_PATTERN) {
		const auto& overlay = d_patternView.Draw(width, height-2);
		WriteXY(out, 0, 1, overlay); }
	else if (d_mode == UM_LOG) {
		const auto& overlay = d_logView.Draw(width, height-2);
		WriteXY(out, 0, 1, overlay); }
	else if (d_mode == UM_HOST) {
		const auto& overlay = d_hostView.Draw(width, height-2);
		WriteXY(out, 0, 1, overlay); }

	if (d_loading) {
		auto attr = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack);
		Fill(out, attr);
		auto [sx, sy] = d_loading->Pack(-1, -1);
		const auto& overlay = d_loading->Draw(sx, sy);
		int xc = (width - overlay.d_width) / 2;
		int yc = (height - overlay.d_height) / 2;
		WriteXY(out, xc, yc, overlay); }

	if (d_splash) {
		auto [sx, sy] = d_splash->Pack(-1, -1);
		const auto& overlay = d_splash->Draw(sx, sy);
		int xc = (width - overlay.d_width) / 2;
		int yc = (height - overlay.d_height) / 2;
		WriteXY(out, xc, yc, overlay); }

	return out; }


const rcls::TextCanvas& RootView::DrawHeader(int width) {
	static rcls::TextCanvas out(width, 1);
	out.Resize(width, 1);
	out.Clear();
	Fill(out, rcls::MakeAttribute(rcls::Color::Red, rcls::Color::Black));
	WriteXY(out, 1, 0, "cxl 0.1.0");
	WriteXY(out, width-9-1, 0, "anix/rqdq");
	return out; }


/*
XXX
const rcls::TextCanvas& RootView::DrawKeyHistory() {
	static rcls::TextCanvas out{ 10, 8 };
	out.Clear();
	Fill(out, rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Blue));
	int row = 0;
	for (const auto& item : d_keyHistory) {
		WriteXY(out, 0, row++, item); }
	return out; }
*/


const rcls::TextCanvas& RootView::DrawTransportIndicator(int width) {
	static rcls::TextCanvas out{ width, 1 };
	out.Clear();
	auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Blue);
	auto hi = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlue);
	auto higreen = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongGreen);
	// auto hired = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongRed);

	//Fill(out, rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlue));
	std::string s;
	WriteXY(out, 1, 0, "Pattern: ", lo);
	WriteXY(out, 10, 0, fmt::sprintf("A-%d", d_unit.GetCurrentPatternNumber()+1), hi);

	WriteXY(out, 14, 0, "| Kit: ", lo);
	WriteXY(out, 21, 0, fmt::sprintf("%d", d_unit.GetCurrentKitNumber()+1), hi);

	int swing = d_unit.GetSwing();
	int tempo = d_unit.GetTempo();
	int whole = tempo/10;
	int tenths = tempo%10;

	WriteXY(out, width-37, 0, "Tempo:", lo);
	s = fmt::sprintf("%3d.%d bpm", whole, tenths);
	WriteXY(out, width-30, 0, s, hi);
	WriteXY(out, width-20, 0, fmt::sprintf("%d%%", swing), swing==50?lo:higreen);
	WriteXY(out, width-16, 0, "|", lo);
	WriteXY(out, width-14, 0, d_unit.IsPlaying() ? "PLAYING" : "STOPPED", d_unit.IsPlaying() ? higreen : lo);
	WriteXY(out, width-6, 0, "|", lo);
	// XXX WriteXY(out, width-4, 0, d_isRecording ? "REC" : "rec", d_isRecording ? hired : lo);
	return out; }


}  // namespace cxl
}  // namespace rqdq
