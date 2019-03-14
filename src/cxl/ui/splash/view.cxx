#include "src/cxl/ui/splash/view.hxx"

#include <array>
#include <iostream>
#include <string>
#include <utility>

#include "src/rcl/rclmt/rclmt_reactor_delay.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace {

constexpr float kAnimRateInHz{30.0f};

constexpr float kSplashDuration{5.0f};

const std::array<std::string, 12> kArtText{ {
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
	R"(                                            )"
	} };


}  // namespace

namespace cxl {

SplashView::SplashView(TextKit::MainLoop& loop) :d_loop(loop) {
	rclmt::Delay(1000.0/kAnimRateInHz, [&]() { Tick(); }); }


std::pair<int, int> SplashView::Pack(int w, int h) {
	return {kArtText[0].size(), kArtText.size()}; }


int SplashView::GetType() {
	return TextKit::WT_FIXED; }


bool SplashView::HandleKeyEvent(TextKit::KeyEvent e) {
	return false; }


const rcls::TextCanvas& SplashView::Draw(int width, int height) {
	const auto mySize = Pack(-1, -1);
	if (width != mySize.first || height != mySize.second) {
		throw std::runtime_error("invalid dimensions given for fixed-size widget"); }

	if (!d_dirty) {
		return d_canvas; }

	d_dirty = false;
	auto& out = d_canvas;
	out.Resize(kArtText[0].size(), kArtText.size());
	out.Clear();
	const auto blue = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongBlue);
	const auto cyan = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::StrongCyan);
	int cyanY = static_cast<int>(d_t*24) % 48;
	for (int y=0; y<kArtText.size(); y++) {
		WriteXY(out, 0, y, kArtText[y], y==cyanY?cyan:blue); }
	return out; }


void SplashView::Tick() {
	// std::cerr << "T" << std::flush;
	d_t += 1/kAnimRateInHz;
	if (d_t > kSplashDuration) {
		onComplete.emit();
		return; }
	d_dirty = true;
	d_loop.DrawScreenEventually();
	rclmt::Delay(1000.0/kAnimRateInHz, [&]() { Tick(); }); }


}  // namespace cxl
}  // namespace rqdq
