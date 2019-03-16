#pragma once
#include <utility>

#include "src/cxl/host.hxx"
#include "src/cxl/ui/host/view.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/mainloop.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {

class HostController {
public:
	HostController(CXLASIOHost& host, TextKit::MainLoop& loop);

private:
	void onHostUpdated();

private:
	CXLASIOHost& d_host;
	TextKit::MainLoop& d_loop;
public:
	HostView d_view; };


}  // namespace cxl
}  // namespace rqdq
