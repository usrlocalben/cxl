#pragma once
#include "src/cxl/unit.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class LoadingStatus : public TextKit::Widget {
public:
	LoadingStatus(CXLUnit& unit);

	// Widget
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;
private:
	void onLoaderStateChange();

private:
	rcls::TextCanvas d_canvas;
	CXLUnit& d_unit;
	bool d_dirty = true; };


}  // namespace cxl
}  // namespace rqdq
