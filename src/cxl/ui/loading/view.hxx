#pragma once
#include "src/cxl/unit.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {


class LoadingView : public TextKit::Widget {
public:
	LoadingView(const CXLUnit& unit);

	// Widget
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

	void Invalidate();

private:
	const CXLUnit& d_unit;

	rcls::TextCanvas d_canvas;
	bool d_dirty = true; };


}  // namespace cxl
}  // namespace rqdq
