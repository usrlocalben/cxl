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
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*w*/, int /*h*/) override;
	int GetType() override;

private:
	bool Refresh();
	const CXLUnit& d_unit;
	float d_pct{0};
	bool d_first{true};
	rcls::TextCanvas d_canvas; };


inline std::shared_ptr<TextKit::Widget> MakeLoadingView(const CXLUnit& unit) {
	return std::make_shared<TextKit::LineBox>(std::make_shared<LoadingView>(unit)); }


}  // namespace cxl
}  // namespace rqdq
