#include "controller.hxx"

#include "src/cxl/host.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

HostController::HostController(CXLASIOHost& host,
                               TextKit::MainLoop& loop)
	:host_(host), loop_(loop), view_{host_} {
	signalId_ = host_.updated_.Connect([&]() { onHostUpdated(); }); }


HostController::~HostController() {
	host_.updated_.Disconnect(signalId_); }


void HostController::onHostUpdated() {
	loop_.DrawScreenEventually(); }


}  // namespace cxl
}  // namespace rqdq
