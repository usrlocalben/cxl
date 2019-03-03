#include "src/cxl/cxl_config.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

#include <algorithm>
#include <iostream>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include "3rdparty/winreg/WinReg.hxx"
#include <Windows.h>

namespace rqdq {

namespace {

std::optional<std::wstring> GetEnvVar(const std::wstring& key) {
	auto keyPtr = key.c_str();
	auto needed = GetEnvironmentVariableW(keyPtr, nullptr, 0);
	if (needed == 0) {
		if (GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
			return {}; }  // not found...
		throw std::runtime_error("GetEnvironmentVariableW failure"); }

	std::wstring buf;
	buf.resize(needed);
	auto stored = GetEnvironmentVariableW(keyPtr, buf.data(), needed);
	if ((stored+1) == needed) {
		buf.resize(needed - 1);  // remove null terminator
		return buf; }
	//std::cout << "needed:" << needed << ", stored: " << stored << "\n";
	throw std::runtime_error("GetEnvironmentVariableW failure: size changed?"); }

}  // namespace

namespace cxl {

namespace config {

std::string asioDriverName = "FlexASIO";

std::string masterLeftDest = "name=OUT 0";

std::string masterRightDest = "name=OUT 1";

std::string dataDir = R"(c:\var\lib\cxl)";
std::string sampleDir = "samples";
std::string patternDir = "patterns";
std::string kitDir = "kits";


void Load() {
	auto key = winreg::RegKey{ HKEY_CURRENT_USER, L"SOFTWARE\\rqdq\\cxl" };

	try {
		asioDriverName = rclt::UTF8Codec::Encode(key.GetStringValue(L"asioDriverName")); }
	catch (const winreg::RegException&) {}

	try {
		masterLeftDest = rclt::UTF8Codec::Encode(key.GetStringValue(L"masterLeftDest")); }
	catch (const winreg::RegException&) {}

	try {
		masterRightDest = rclt::UTF8Codec::Encode(key.GetStringValue(L"masterRightDest")); }
	catch (const winreg::RegException&) {}

	try {
		dataDir = rclt::UTF8Codec::Encode(key.GetStringValue(L"dataDir")); }
	catch (const winreg::RegException&) {}

	auto appDataDir = rclt::UTF8Codec::Encode(GetEnvVar(L"APPDATA").value_or(L"%APPDATA%"));
	//std::cout << ">>> APPDATA = [" << appDataDir << "]\n";
	auto mod = std::regex_replace(dataDir, std::regex("%APPDATA%", std::regex::icase), appDataDir);
	dataDir = mod;

	sampleDir = dataDir + "\\" + "samples";
	patternDir = dataDir + "\\" + "patterns";
	kitDir = dataDir + "\\" + "kits";
}
	//std::cout << ">>> dataDir = [" << dataDir << "]\n"; }



}  // namespace config

}  // namespace cxl
}  // namespace rqdq
