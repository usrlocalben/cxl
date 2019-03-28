#include "log.hxx"

#include <fstream>
#include <string>

#include "src/cxl/config.hxx"
#include "src/rcl/rcls/rcls_file.hxx"

namespace rqdq {
namespace {

std::string logPath;

}  // namespace

namespace cxl {

Log::Log() {
	logPath = rcls::JoinPath(cxl::config::dataDir, "message.log");
	buf_.resize(1024); }


Log& Log::GetInstance() {
	static Log log;
	return log; }


void Log::warn(const std::string& msg) {
	return info(msg); }


void Log::info(const std::string& msg) {
	// std::ofstream fd(logPath, std::ios_base::app);
	// fd << msg << "\n";

	buf_[head_].assign(msg);
	head_++;
	if (head_ >= 1024) {
		head_ = 0; }
	updated_.Emit(); }


}  // namespace cxl
}  // namespace rqdq
