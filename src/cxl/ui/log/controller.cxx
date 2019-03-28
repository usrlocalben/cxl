#include "controller.hxx"

#include "src/cxl/log.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

LogController::LogController(TextKit::MainLoop& loop) :loop_(loop) {
	auto& log = Log::GetInstance();
	signalId_ = log.updated_.Connect([&]() { onLogWrite(); }); }


void LogController::onLogWrite() {
	loop_.DrawScreenEventually(); }


LogController::~LogController() {
	auto& log = Log::GetInstance();
	log.updated_.Disconnect(signalId_); }


}  // namespace cxl
}  // namespace rqdq
