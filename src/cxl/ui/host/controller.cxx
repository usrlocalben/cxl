#include "controller.hxx"

#include "src/cxl/host.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

HostController::HostController(CXLASIOHost& host,
                               TextKit::MainLoop& loop)
	:d_host(host), d_loop(loop), d_view{d_host} {
	d_signalId = d_host.d_updated.Connect([&]() { onHostUpdated(); }); }


HostController::~HostController() {
	d_host.d_updated.Disconnect(d_signalId); }


void HostController::onHostUpdated() {
	d_loop.DrawScreenEventually(); }


}  // namespace cxl
}  // namespace rqdq
