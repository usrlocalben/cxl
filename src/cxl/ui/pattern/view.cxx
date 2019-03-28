#include "view.hxx"

#include <array>
#include <string>
#include <string_view>
#include <utility>

#include "src/cxl/ui/pattern/page.hxx"
#include "src/cxl/ui/pattern/state.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace {

constexpr std::array kTrackNames = {
	"BD", "SD", "HT", "MD",
	"LT", "CP", "RS", "CB",
	"CH", "OH", "RC", "CC",
	"M1", "M2", "M3", "M4" };


char tolower(char ch) {
	if ('A' <= ch && ch <= 'Z') {
		return ch - 'A' + 'a'; }
	return ch; }


std::string_view tolower(std::string_view s) {
	thread_local std::string tmp;
	tmp.clear();
	for (auto ch : s) {
		tmp.push_back(tolower(ch)); }
	return tmp.data(); }

}  // namespace

namespace cxl {


PatternView::PatternView(const CXLUnit& unit, const EditorState& state)
	:unit_(unit), state_(state) {}


std::pair<int, int> PatternView::Pack(int w, int h) {
	return {80, 23}; }


int PatternView::GetType() {
	return TextKit::WT_BOX; }


const rcls::TextCanvas& PatternView::Draw(int width, int height) {
	auto& out = canvas_;
	out.Resize(width, height);
	out.Clear();
	WriteXY(out, 2, 4, DrawTrackSelection());
	WriteXY(out, 16, 3, DrawParameters());

	WriteXY(out, 1, height-3, DrawGrid());
	WriteXY(out, 68, height-2, DrawPageIndicator());

	if (popup_) {
		auto attr = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack);
		Fill(out, attr);
		auto [sx, sy] = popup_->Pack(-1, -1);
		const auto& overlay = popup_->Draw(sx, sy);
		int xc = (width - overlay.d_width) / 2;
		int yc = (height - overlay.d_height) / 2;
		WriteXY(out, xc, yc, overlay); }

	return out; }


const rcls::TextCanvas& PatternView::DrawTrackSelection() {
	static rcls::TextCanvas out{ 13, 4 };
	out.Clear();
	auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack);
	auto hi = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlue);
	for (int ti = 0; ti < 4; ti++) {
		auto isMuted = unit_.IsTrackMuted(ti);
		WriteXY(out, (ti- 0)*3+1, 0, tolower(kTrackNames[ti]), isMuted?lo:hi); }
	for (int ti = 4; ti < 8; ti++) {
		auto isMuted = unit_.IsTrackMuted(ti);
		WriteXY(out, (ti- 4)*3+1, 1, tolower(kTrackNames[ti]), isMuted?lo:hi); }
	for (int ti = 8; ti <12; ti++) {
		auto isMuted = unit_.IsTrackMuted(ti);
		WriteXY(out, (ti- 8)*3+1, 2, tolower(kTrackNames[ti]), isMuted?lo:hi); }
	for (int ti =12; ti <16; ti++) {
		auto isMuted = unit_.IsTrackMuted(ti);
		WriteXY(out, (ti-12)*3+1, 3, tolower(kTrackNames[ti]), isMuted?lo:hi); }

	int selY = state_.curTrack / 4;
	int selX = (state_.curTrack % 4) * 3 + 1;
	auto isMuted = unit_.IsTrackMuted(state_.curTrack);
	WriteXY(out, selX-1, selY, fmt::sprintf("[%s]", kTrackNames[state_.curTrack]), isMuted?lo:hi);
	return out; }


const rcls::TextCanvas& PatternView::DrawParameters() {
	static rcls::TextCanvas out{ 7*4, 1+2*2+1 };
	out.Clear();
	Fill(out, rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Cyan));

	auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Brown);
	auto hi = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBrown);

	auto inactive = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Cyan);
	auto active = rcls::MakeAttribute(rcls::Color::Cyan, rcls::Color::Black);

	WriteXY(out, 0, 0, "Voice", state_.curVoicePage==0?hi:lo);
	WriteXY(out, 7, 0, "Effect", state_.curVoicePage==1?hi:lo);
	WriteXY(out, 15, 0, "Mix", state_.curVoicePage==2?hi:lo);

	// row 1
	int page = state_.curVoicePage;

	int row = 0;
	for (int pi=0; pi<4; pi++) {
		const auto paramName = GetPageParameterName(unit_, page, state_.curTrack, pi);
		if (!paramName.empty()) {
			int value = GetPageParameterValue(unit_, page, state_.curTrack, pi);
			WriteXY(out, 7*pi+2, row*2+0+1, paramName, state_.curParam.value_or(-1) == pi ? active : inactive);
			WriteXY(out, 7*pi+0, row*2+1+1, fmt::sprintf("% 3d", value));}}

	// row 2
	row = 1;
	for (int pi=4; pi<8; pi++) {
		const auto paramName = GetPageParameterName(unit_, page, state_.curTrack, pi);
		if (!paramName.empty()) {
			int value = GetPageParameterValue(unit_, page, state_.curTrack, pi);
			WriteXY(out, 7*(pi-4)+2, row*2+0+1, paramName, state_.curParam.value_or(-1) == pi ? active : inactive);
			WriteXY(out, 7*(pi-4)+0, row*2+1+1, fmt::sprintf("% 3d", value));}}

	int waveId = unit_.GetVoiceParameterValue(state_.curTrack, 0);
	auto waveName = unit_.GetWaveName(waveId);
	WriteXY(out, 0, 4+1, fmt::sprintf("Wave: %s", waveName));
	return out; }


const rcls::TextCanvas& PatternView::DrawGrid() {
	static rcls::TextCanvas out{ 65, 3 };
	auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Brown);
	auto hi = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBrown);
	auto red = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongRed);
	auto dark = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlack);
	Fill(out, lo);
	WriteXY(out, 0, 0, "| .   .   .   . | .   .   .   . | .   .   .   . | .   .   .   . | ");
	const int curPos = unit_.GetPlayingNoteIndex();
	const int curPage = curPos/16;
	const int curPagePos = curPos%16;

	if (curPage == state_.curGridPage) {
		WriteXY(out, 2+curPagePos*4, 0, "o", hi); }

	WriteXY(out, 0, 1, "|");
	for (int i = 0; i < 16; i++) {
		auto value = unit_.GetTrackGridNote(state_.curTrack, state_.curGridPage*16+i);
		WriteXY(out, i*4+2, 1, value != 0 ? "X" : " ", red);
		WriteXY(out, i*4+4, 1, "|", lo); }

	if (state_.isRecording) {
		WriteXY(out, 0, 2, "REC", red); }
	else {
		WriteXY(out, 0, 2, "rec", dark); }
	return out; }


const rcls::TextCanvas& PatternView::DrawPageIndicator() {
	static rcls::TextCanvas out{ 9, 1 };
	int curPage = state_.curGridPage;
	int playingPage = unit_.GetPlayingNoteIndex() / 16;

	auto lo = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Brown);
	auto hi = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBrown);
	auto red = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongRed);
	Fill(out, lo);
	WriteXY(out, 0, 0, "[. . . .]", lo);

	for (int n=0; n<4; n++) {
		if (unit_.IsPlaying() && playingPage == n) {
			WriteXY(out, n*2+1, 0, "o", red); }
		else if (curPage == n) {
			WriteXY(out, n*2+1, 0, "o", hi); }
		else {}}

	return out; }


}  // namespace cxl
}  // namespace rqdq
