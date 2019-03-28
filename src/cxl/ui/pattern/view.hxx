#pragma once
#include <memory>
#include <utility>

#include "src/cxl/unit.hxx"
#include "src/cxl/ui/pattern/state.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {

class PatternView : public TextKit::Widget {
public:
	PatternView(const CXLUnit& /*unit*/, const EditorState& /*state*/);

	// Widget impl
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*w*/, int /*h*/) override;
	int GetType() override;

private:
	const rcls::TextCanvas& DrawTrackSelection();
	const rcls::TextCanvas& DrawParameters();
	const rcls::TextCanvas& DrawGrid();
	const rcls::TextCanvas& DrawPageIndicator();

private:
	const CXLUnit& unit_;
	const EditorState& state_;

public:
    std::shared_ptr<TextKit::Widget> popup_;

private:
	rcls::TextCanvas canvas_; };


}  // namespace cxl
}  // namespace rqdq
