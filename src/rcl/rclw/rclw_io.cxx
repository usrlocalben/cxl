#include "src/rcl/rclw/rclw_io.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

#include <cstring>
#include <stdexcept>
#include <string>
#include <Windows.h>

namespace rqdq {
namespace rclw {

void EnsureOpenable(const std::wstring& path) {
	const auto tmp = rclt::UTF8Codec::Encode(path);
	const char* pathPtr = tmp.c_str();
	OFSTRUCT ofs;
	HFILE hfile;
	memset(&ofs, 0, sizeof(OFSTRUCT));
	ofs.cBytes = sizeof(OFSTRUCT);
	hfile = OpenFile(pathPtr, &ofs, OF_EXIST);
	if (!hfile) {
		throw std::runtime_error("file is not openable");}}

}  // close package namespace
}  // close enterprise namespace
