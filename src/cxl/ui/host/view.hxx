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
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	const CXLASIOHost& d_host;
	rcls::TextCanvas d_canvas; };


}  // namespace cxl
}  // namespace rqdq
