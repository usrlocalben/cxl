/**
 * urwid-inspired Widget
 */
#pragma once
#include "src/rcl/rclw/rclw_console_canvas.hxx"
#include "src/textkit/keyevent.hxx"

#include <memory>
#include <optional>

#include <Windows.h>

namespace rqdq {
namespace TextKit {

const int WT_FIXED = 1;
const int WT_BOX = 2;
const int WT_FLOW = 4;

class Widget {
public:
	virtual bool HandleKeyEvent(KeyEvent) = 0;
	virtual const rclw::ConsoleCanvas& Draw(int width, int height) = 0;
	virtual std::pair<int, int> Pack(int, int) = 0;
	virtual int GetType() = 0;
	virtual ~Widget() = default; };


class LineBox : public Widget {
public:
	LineBox(std::shared_ptr<Widget> widget);
	LineBox(std::shared_ptr<Widget> widget, const std::string& title);

	// Widget
	bool HandleKeyEvent(KeyEvent) override;
	const rclw::ConsoleCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

	std::shared_ptr<Widget> d_widget;
	std::optional<std::string> d_title = {};
	rclw::ConsoleCanvas d_canvas; };


}  // namespace TextKit
}  // namespace rqdq
