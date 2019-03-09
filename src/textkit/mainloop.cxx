#include "src/textkit/mainloop.hxx"

#include "src/rcl/rclmt/rclmt_event.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_reactor_keyboard.hxx"

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include "3rdparty/fmt/include/fmt/printf.h"
#include <Windows.h>

namespace rqdq {
namespace TextKit {

MainLoop::MainLoop(rclmt::Reactor* reactor_/*=nullptr*/)
	:d_reactor(reactor_ != nullptr? *reactor_: rclmt::Reactor::GetInstance()) {}


void MainLoop::DrawScreen() {
	if (d_widget != nullptr) {
		auto canvas = d_widget->Draw(80, 25);
		SMALL_RECT rect;
		rect.Left = 0;
		rect.Top = 0;
		rect.Right = canvas.d_width - 1;
		rect.Bottom = canvas.d_height - 1;
		auto canvasData = reinterpret_cast<CHAR_INFO*>(canvas.GetDataPtr());
		auto result = WriteConsoleOutputW(GetStdHandle(STD_OUTPUT_HANDLE),
		                                  canvasData,
		                                  COORD{ short(canvas.d_width), short(canvas.d_height) },
		                                  COORD{ 0, 0 },
		                                  &rect);
		if (result == 0) {
			throw std::runtime_error("WriteConsoleOutput failure"); }}}


void MainLoop::DrawScreenEventually() {
	d_redrawEvent.Signal(); }


void MainLoop::onInput(rclmt::KeyEvent key) {
	if (d_widget != nullptr) {
		bool handled = d_widget->HandleKeyEvent(key);
		if (handled) {
			DrawScreen(); }}}


void MainLoop::Run() {
	d_reactor.ListenMany(d_redrawEvent, [&](){ DrawScreen(); });
	rclmt::ListenKeyboard([&](rclmt::KeyEvent e) { onInput(e); }, &d_reactor);
	d_reactor.Run(); }


bool MainLoop::IsKeyDown(int scanCode) {
	return rqdq::rclmt::IsKeyDown(scanCode); }


}  // namespace TextKit
}  // namespace rqdq
