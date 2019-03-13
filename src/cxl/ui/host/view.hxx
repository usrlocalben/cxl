#pragma once
#include <utility>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/keyevent.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"
#include "src/cxl/host.hxx"

namespace rqdq {
namespace cxl {

class HostView : public TextKit::Widget {
public:
	HostView(CXLASIOHost& host, TextKit::MainLoop& loop);

	void onHostUpdated();

	// Widget impl
	bool HandleKeyEvent(TextKit::KeyEvent) override;
	const rcls::TextCanvas& Draw(int, int) override;
	std::pair<int, int> Pack(int, int) override;
	int GetType() override;

private:
	CXLASIOHost& d_host;
	TextKit::MainLoop& d_loop;
	rcls::TextCanvas d_canvas; };


}  // namespace cxl
}  // namespace rqdq
