#include "src/cxl/log.hxx"

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
	d_buf.resize(1024); }


Log& Log::GetInstance() {
	static Log log;
	return log; }


void Log::warn(const std::string& msg) {
	return info(msg); }


void Log::info(const std::string& msg) {
	// std::ofstream fd(logPath, std::ios_base::app);
	// fd << msg << "\n";

	d_buf[d_head].assign(msg);
	d_head++;
	if (d_head >= 1024) {
		d_head = 0; }
	d_updated.emit(); }


}  // namespace cxl
}  // namespace rqdq
