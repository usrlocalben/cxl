#include <algorithm>
#include <array>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "src/cxl/config.hxx"
#include "src/cxl/host.hxx"
#include "src/cxl/log.hxx"
#include "src/cxl/ui/root/controller.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rclmt/rclmt_reactor_delay.hxx"
#include "src/rcl/rcls/rcls_console.hxx"
#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

using namespace std;


int main(int argc, char **argv) {
	config::Load();
	auto& log = Log::GetInstance();
	log.info("Log started");

	const array<string, 3> userDirs = {config::patternDir, config::sampleDir, config::kitDir};
	for_each(userDirs.begin(), userDirs.end(), rcls::EnsureDirectoryExists);

	CXLUnit unit;

	CXLASIOHost host{unit};

	rclmt::Delay(0, [&]() {
		if (host.SetDriver(config::asioDriverName)) {
			const std::array<std::string, 2> chans = { config::masterLeftDest, config::masterRightDest };
			for (int ci=0; ci<2; ci++) {
				string mlc = chans[ci];
				string msg;
				if (rclt::ConsumePrefix(mlc, "name=")) {
					// identify connection by channel name
					host.SetChannel(config::asioDriverName, ci, mlc); }
				/*else if (rclt::ConsumePrefix(mlc, "num=")) {
					// identify connection by index
					const int num = stoi(mlc);
					success = host.AttachChannel(ci, num);
					msg = fmt::sprintf("asio channel number %d not found", num); }*/
				else {
					msg = fmt::sprintf("invalid asio connection \"%s\" expected "
									   "either num=<num> or name=<text>",
									   chans[ci]);}
				if (!msg.empty()) {
					log.info(msg); }}
			host.Start(); }
		else {
			auto msg = fmt::sprintf("ASIO driver \"%s\" not available.",
									config::asioDriverName);
			log.info(msg); }});

	auto& console = rcls::Console::GetInstance();
	console.SetDimensions(80, 25);
	console.Clear();

	RootController(unit, host).Run();

	for (int n=0; n<25; n++) {
		cout << "\n"; }
	cout << "CXL Session Ended.\n";
	return 0; }


}  // namespace cxl
}  // namespace rqdq


int main(int argc, char **argv) {
	int result;
	try {
		result = rqdq::cxl::main(argc, argv); }
	catch (const std::exception& err) {
		std::cerr << "exception: " << err.what() << std::endl;
		result = EXIT_FAILURE; }
	catch (...) {
		std::cerr << "unknown exception" << std::endl;
		result = EXIT_FAILURE; }
	return result; }
