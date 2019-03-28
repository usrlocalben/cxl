/**
 * urwid-inspired Widget
 */
#pragma once
#include <memory>
#include <optional>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"

namespace rqdq {
namespace TextKit {

constexpr int WT_FIXED = 1;
constexpr int WT_BOX = 2;
constexpr int WT_FLOW = 4;

class Widget {
public:
	virtual bool HandleKeyEvent(KeyEvent /*unused*/) {
		return false; }
	virtual const rcls::TextCanvas& Draw(int width, int height) = 0;
	virtual std::pair<int, int> Pack(int, int) = 0;
	virtual int GetType() = 0;
	virtual ~Widget() = default; };


class LineBox : public Widget {
public:
	explicit LineBox(Widget* widget);
	explicit LineBox(std::shared_ptr<Widget> widget);
	LineBox(std::shared_ptr<Widget> widget, const std::string& title);

	// Widget
	bool HandleKeyEvent(KeyEvent /*e*/) override;
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*width*/, int /*height*/) override;
	int GetType() override;

	std::shared_ptr<Widget> d_widget;
	std::optional<std::string> d_title = {};
	rcls::TextCanvas d_canvas; };


}  // namespace TextKit
}  // namespace rqdq
