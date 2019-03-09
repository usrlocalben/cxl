#include "src/textkit/widget.hxx"

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"

#include <memory>
#include <string>
#include <utility>

namespace rqdq {
namespace TextKit {


LineBox::LineBox(std::shared_ptr<Widget> widget)
	:d_widget(std::move(widget)) {}


LineBox::LineBox(std::shared_ptr<Widget> widget, const std::string& title)
	:d_widget(std::move(widget)), d_title(title) {}


bool LineBox::HandleKeyEvent(KeyEvent e) {
	return d_widget->HandleKeyEvent(e); }


std::pair<int, int> LineBox::Pack(int width, int height) {
	auto [w, h] = d_widget->Pack(width, height);
	return {w+2, h+2};}


int LineBox::GetType() {
	return d_widget->GetType(); }


const rcls::TextCanvas& LineBox::Draw(int width, int height) {
	auto& out = d_canvas;
	out.Resize(width, height);
	auto xa = rcls::MakeAttribute(rcls::Color::Black, rcls::Color::Blue);
	const auto& sub = d_widget->Draw(width-2, height-2);
	WriteXY(out, 0, 0, "/--- -  -", xa);
	WriteXY(out, width-1, 0, "\\", xa);
	WriteXY(out, 0, height-1, "\\", xa);
	WriteXY(out, width-5, height-1, "- --/", xa);
	WriteXY(out, 1, 1, sub);
	return out; }


}  // namespace TextKit
}  // namespace rqdq
