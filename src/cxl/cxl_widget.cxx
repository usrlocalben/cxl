#include "src/cxl/cxl_widget.hxx"
#include "src/rcl/rclw/rclw_console_canvas.hxx"

#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace rqdq {
namespace cxl {


LineBox::LineBox(std::shared_ptr<Widget> widget)
	:d_widget(std::move(widget)) {}


LineBox::LineBox(std::shared_ptr<Widget> widget, const std::string& title)
	:d_widget(std::move(widget)), d_title(title) {}


bool LineBox::HandleKeyEvent(KEY_EVENT_RECORD e) {
	return d_widget->HandleKeyEvent(e); }


std::pair<int, int> LineBox::Pack(int width, int height) {
	auto [w, h] = d_widget->Pack(width, height);
	return {w+2, h+2};}


int LineBox::GetType() {
	return d_widget->GetType(); }


const rclw::ConsoleCanvas& LineBox::Draw(int width, int height) {
	auto& out = d_canvas;
	out.Resize(width, height);
	auto xa = rclw::MakeAttribute(rclw::Color::Black, rclw::Color::Blue);
	const auto& sub = d_widget->Draw(width-2, height-2);
	WriteXY(out, 0, 0, "/--- -  -", xa);
	WriteXY(out, width-1, 0, "\\", xa);
	WriteXY(out, 0, height-1, "\\", xa);
	WriteXY(out, width-5, height-1, "- --/", xa);
	WriteXY(out, 1, 1, sub);
	return out; }


}  // namespace cxl
}  // namespace rqdq
