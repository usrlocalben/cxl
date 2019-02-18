#include "src/cxl/cxl_unit_view.hxx"
#include "src/cxl/cxl_unit.hxx"
#include "src/rcl/rclw/rclw_console.hxx"

#include <array>
#include <deque>
#include <string>
#include <sstream>


namespace rqdq {
namespace {

const std::array<const std::string, 16> kTrackNames = {
	"BD", "SD", "HT", "MD", "LT", "CP", "RS", "CB",
	"CH", "OH", "RC", "CC", "M1", "M2", "M3", "M4" };


char tolower(char ch) {
	if ('A' <= ch && ch <= 'Z') {
		return ch - 'A' + 'a'; }
	return ch; }


const std::string& tolower(const std::string& s) {
	thread_local std::string tmp;
	tmp.clear();
	for (auto ch : s) {
		tmp.push_back(tolower(ch)); }
	return tmp; }

}  // namespace

namespace cxl {


CXLUnitView::CXLUnitView(
	CXLUnit& unit,
	int selectedTrack,
	int selectedPage,
	std::deque<std::string>& keyHistory,
	bool enableKeyDebug)
	:d_unit(unit),
	d_selectedTrack(selectedTrack),
	d_selectedPage(selectedPage),
	d_keyHistory(keyHistory),
	d_enableKeyDebug(enableKeyDebug)
	{}


void CXLUnitView::Draw(rclw::Console& console) {
	console
		.Position(1,0).Write("cxl 0.1.0")
		.Position(79-10,0).Write("anix/rqdq");
	DrawTrackSelection(console, 0, 2);
	DrawGrid(console, 1, 21);
	if (d_enableKeyDebug) {
		DrawKeyHistory(console, 60, 10); }
	DrawParameters(console, 8, 6);
	DrawTransportIndicator(console); }


void CXLUnitView::DrawTrackSelection(rclw::Console& console, int x, int y) {
	console
		.Position(x, y)
		.LeftEdge(x);
	console.Write("  ");
	for (int i = 0; i < 8; i++) {
		console.Write(tolower(kTrackNames[i]) + " ");}
	console.CR();
	console.Write("  ");
	for (int i = 8; i <16; i++) {
		console.Write(tolower(kTrackNames[i]) + " ");}

	int selY = d_selectedTrack / 8       +  y;
	int selX = (d_selectedTrack % 8) * 3 + (x+2);
	console.Position(selX, selY).Write(kTrackNames[d_selectedTrack]);
	console.Position(selX-1, selY).Write("[");
	console.Position(selX+2, selY).Write("]"); }


void CXLUnitView::DrawParameters(rclw::Console& console, int x, int y) {
	console.Position(x, y).LeftEdge(x);
	for (int i = 0; i < 8; i++) {
		int paramNum = d_selectedPage*8+i;
		auto& paramName = d_unit.GetVoiceParameterName(d_selectedTrack, paramNum);
		int value = d_unit.GetVoiceParameterValue(d_selectedTrack, paramNum);
		std::stringstream ss;
		ss << paramName << ": " << value << "      ";
		console.Write(ss.str()).CR(); }
	int waveId = d_unit.GetVoiceParameterValue(d_selectedTrack, 4);
	std::string waveName = d_unit.GetWaveName(waveId);
	console.Position(20,10).Write(waveName + "         ");}


void CXLUnitView::DrawGrid(rclw::Console& console, int x, int y) {
	console.Position(x, y);
	console.Write("| .   .   .   . | .   .   .   . | .   .   .   . | .   .   .   . | ");
	int pos = d_unit.GetLastPlayedGridPosition();
	console.Position(x+2 + pos*4, y).Write("o");
	console.Position(1, y+1);
	console.Write("| ");
	for (int i = 0; i < 16; i++) {
		auto value = d_unit.GetTrackGridNote(d_selectedTrack, i);
		console.Write(value != 0 ? "X" : " ");
		console.Write(" | "); }}


void CXLUnitView::DrawKeyHistory(rclw::Console& console, int x, int y) {
	console.Position(x, y).LeftEdge(x);
	for (const auto& item : d_keyHistory) {
		console.Write(item).CR(); }}


void CXLUnitView::DrawTransportIndicator(rclw::Console& console) {
	int tempo = d_unit.GetTempo();
	int whole = tempo/10;
	int tenths = tempo%10;
	std::stringstream ss;
	ss << "Tempo: " << whole << "." << tenths << " bpm | ";
	console.Position(53, 24).Write(ss.str());
	console.Position(79-8, 24).Write(d_unit.IsPlaying() ? "PLAYING" : "STOPPED"); }


}  // namespace cxl
}  // namespace rqdq
