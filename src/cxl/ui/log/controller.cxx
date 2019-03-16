#include "src/cxl/ui/log/controller.hxx"

#include "src/cxl/log.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

LogController::LogController(TextKit::MainLoop& loop) :d_loop(loop) {
	auto& log = Log::GetInstance();
	log.d_updated.connect([&]() { onLogWrite(); }); }


void LogController::onLogWrite() {
	d_loop.DrawScreenEventually(); }


}  // namespace cxl
}  // namespace rqdq
