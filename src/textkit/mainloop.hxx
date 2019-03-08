#pragma once
#include "src/rcl/rclmt/rclmt_deferred.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

#include <functional>
#include <iostream>
#include <vector>

#include <Windows.h>

namespace rqdq {
namespace TextKit {


class MainLoop {
public:
	MainLoop(rclmt::Reactor* reactor_=nullptr);

	void Run();
	void Stop();

	bool IsKeyDown(int scanCode);

	void DrawScreen();
	void DrawScreenEventually();

private:
	void onInput(rclmt::KeyEvent key);

public:
	Widget* d_widget = nullptr;
private:
	rclmt::Reactor& d_reactor;
	rclmt::Event d_redrawEvent = rclmt::Event::MakeEvent(); };


}  // namespace cxl
}  // namespace rqdq
