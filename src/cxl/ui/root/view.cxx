#include "view.hxx"

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
	:unit_(unit),
	hostView_(hostView),
	patternView_(patternView),
	logView_(logView),
	mode_(mode) {}


std::pair<int, int> RootView::Pack(int w, int h) {
	return {80, 25}; }


int RootView::GetType() {
	return TextKit::WT_BOX; }


const rcls::TextCanvas& RootView::Draw(int width, int height) {
	auto& out = canvas_;
	out.Resize(width, height);
	out.Clear();
	WriteXY(out, 0, 0, DrawHeader(width));

	WriteXY(out, 0, height-1, DrawTransportIndicator(width));

	if (mode_ == UM_PATTERN) {
		const auto& overlay = patternView_.Draw(width, height-2);
		WriteXY(out, 0, 1, overlay); }
	else if (mode_ == UM_LOG) {
		const auto& overlay = logView_.Draw(width, height-2);
		WriteXY(out, 0, 1, overlay); }
	else if (mode_ == UM_HOST) {
		const auto& overlay = hostView_.Draw(width, height-2);
		WriteXY(out, 0, 1, overlay); }

	if (loading_) {
		auto attr = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack);
		Fill(out, attr);
		auto [sx, sy] = loading_->Pack(-1, -1);
		const auto& overlay = loading_->Draw(sx, sy);
		int xc = (width - overlay.d_width) / 2;
		int yc = (height - overlay.d_height) / 2;
		WriteXY(out, xc, yc, overlay); }

	if (splash_) {
		auto [sx, sy] = splash_->Pack(-1, -1);
		const auto& overlay = splash_->Draw(sx, sy);
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
	WriteXY(out, 10, 0, fmt::sprintf("A-%d", unit_.GetCurrentPatternNumber()+1), hi);

	WriteXY(out, 14, 0, "| Kit: ", lo);
	WriteXY(out, 21, 0, fmt::sprintf("%d", unit_.GetCurrentKitNumber()+1), hi);

	int swing = unit_.GetSwing();
	int tempo = unit_.GetTempo();
	int whole = tempo/10;
	int tenths = tempo%10;

	WriteXY(out, width-37, 0, "Tempo:", lo);
	s = fmt::sprintf("%3d.%d bpm", whole, tenths);
	WriteXY(out, width-30, 0, s, hi);
	WriteXY(out, width-20, 0, fmt::sprintf("%d%%", swing), swing==50?lo:higreen);
	WriteXY(out, width-16, 0, "|", lo);
	WriteXY(out, width-14, 0, unit_.IsPlaying() ? "PLAYING" : "STOPPED", unit_.IsPlaying() ? higreen : lo);
	WriteXY(out, width-6, 0, "|", lo);
	// XXX WriteXY(out, width-4, 0, d_isRecording ? "REC" : "rec", d_isRecording ? hired : lo);
	return out; }


}  // namespace cxl
}  // namespace rqdq
