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
	~HostController();
	HostController& operator=(const HostController& other) = delete;
	HostController(const HostController& other) = delete;

private:
	void onHostUpdated();

private:
	CXLASIOHost& host_;
	TextKit::MainLoop& loop_;
	int signalId_{-1};
public:
	HostView view_;};


}  // namespace cxl
}  // namespace rqdq
