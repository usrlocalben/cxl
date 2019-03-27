#include "controller.hxx"

#include "src/cxl/log.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

LogController::LogController(TextKit::MainLoop& loop) :d_loop(loop) {
	auto& log = Log::GetInstance();
	d_signalId = log.d_updated.Connect([&]() { onLogWrite(); }); }


void LogController::onLogWrite() {
	d_loop.DrawScreenEventually(); }


LogController::~LogController() {
	auto& log = Log::GetInstance();
	log.d_updated.Disconnect(d_signalId); }


}  // namespace cxl
}  // namespace rqdq
