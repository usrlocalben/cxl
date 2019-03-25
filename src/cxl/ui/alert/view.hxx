#pragma once
#include <memory>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class Alert : public TextKit::Widget {
public:
	Alert(std::string text);

	// Widget
	bool HandleKeyEvent(TextKit::KeyEvent /*e*/) override;
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*w*/, int /*h*/) override;
	int GetType() override;

private:
	bool Refresh();
	std::string d_text;
	rcls::TextCanvas d_canvas;
	bool d_first{true}; };


inline std::shared_ptr<TextKit::Widget> MakeAlert(const std::string& text) {
	return std::make_shared<TextKit::LineBox>(std::make_shared<Alert>(text)); }


}  // namespace cxl
}  // namespace rqdq

