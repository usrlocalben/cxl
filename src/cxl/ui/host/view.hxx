#pragma once
#include <utility>

#include "src/cxl/host.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {

class HostView : public TextKit::Widget {
public:
	HostView(CXLASIOHost& host);

	// Widget impl
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*w*/, int /*h*/) override;
	int GetType() override;

private:
	const CXLASIOHost& host_;
	rcls::TextCanvas canvas_; };


}  // namespace cxl
}  // namespace rqdq
