#include "src/cxl/ui/host/controller.hxx"

#include "src/cxl/host.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

HostController::HostController(CXLASIOHost& host,
                               TextKit::MainLoop& loop)
	:d_host(host), d_loop(loop), d_view{d_host} {
	d_host.d_updated.connect([&]() { onHostUpdated(); }); }


void HostController::onHostUpdated() {
	d_loop.DrawScreenEventually(); }


}  // namespace cxl
}  // namespace rqdq
