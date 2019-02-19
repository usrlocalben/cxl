#include "src/cxl/cxl_config.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

#include <algorithm>
#include <string>
#include "3rdparty/winreg/winreg.hxx"

namespace rqdq {
namespace cxl {

namespace config {

std::string asioDriverName = "FlexASIO";

std::string masterLeftDest = "name=OUT 0";

std::string masterRightDest = "name=OUT 1";

std::string dataDir = R"(c:\var\lib\cxl)";

void Load() {
	auto key = winreg::RegKey{ HKEY_CURRENT_USER, L"SOFTWARE\\rqdq\\cxl" };

	try {
		asioDriverName = rclt::UTF8Codec::encode(key.GetStringValue(L"asioDriverName")); }
	catch (const winreg::RegException& err) { err; }

	try {
		masterLeftDest = rclt::UTF8Codec::encode(key.GetStringValue(L"masterLeftDest")); }
	catch (const winreg::RegException& err) { err; }

	try {
		masterRightDest = rclt::UTF8Codec::encode(key.GetStringValue(L"masterRightDest")); }
	catch (const winreg::RegException& err) { err; }

	try {
		dataDir = rclt::UTF8Codec::encode(key.GetStringValue(L"dataDir")); }
	catch (const winreg::RegException& err) { err; }
	}

}  // namespace config

}  // namespace cxl
}  // namespace rqdq
