#include "view.hxx"

#include <array>
#include <stdexcept>
#include <string>
#include <utility>

#include "src/rcl/rcls/rcls_text_canvas.hxx"

namespace rqdq {
namespace {

constexpr std::array kArtText = {
	R"(                                            )",
	R"(                        __                  )",
	R"(                 ____  /  \                 )",
	R"(     .---------_/    \/    \---.            )",
	R"(    |     _____/\__     ___/   |______ _    )",
	R"(    |         /_.         \_   \     // /   )",
	R"(     -- _____/\      /_____/________//_/    )",
	R"(               \____/                       )",
	R"(                                            )",
	R"(                  cxl  v1.0                 )",
	R"(                  f1 = help                 )",
	R"(                                            )" };

constexpr int kArtHeight = kArtText.size();

constexpr int kArtWidth = std::char_traits<char>::length(kArtText[0]);


}  // namespace

namespace cxl {

SplashView::SplashView(const float& t) :d_tSrc(t) {}


std::pair<int, int> SplashView::Pack(int w, int h) {
	return {kArtWidth, kArtHeight}; }


int SplashView::GetType() {
	return TextKit::WT_FIXED; }


bool SplashView::Refresh() {
	bool updated = false;
	if (d_tSrc != d_t) {
		updated = true;
		d_t = d_tSrc; }
	return updated; }


const rcls::TextCanvas& SplashView::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	auto& out = d_canvas;
	bool updated = Refresh();
	if (updated) {
		out.Resize(mySize.first, mySize.second);
		out.Clear();
		const auto blue = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlue);
		const auto cyan = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongCyan);
		int cyanY = static_cast<int>(d_t*24) % 48;
		for (int y=0; y<kArtHeight; y++) {
			WriteXY(out, 0, y, kArtText[y], y==cyanY?cyan:blue); }}
	return out; }


}  // namespace cxl
}  // namespace rqdq
