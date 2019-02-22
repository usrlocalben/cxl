#pragma once
#include "src/rcl/rclw/rclw_console_canvas.hxx"

#define NOMINMAX
#include <Windows.h>

namespace rqdq {
namespace cxl {

/**
 * urwid-inspired Widget interface
 */
class Widget {
public:
	virtual bool HandleKeyEvent(KEY_EVENT_RECORD) = 0;
	virtual rclw::ConsoleCanvas Draw() = 0; };


}  // namespace cxl
}  // namespace rqdq
