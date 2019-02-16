#pragma once
#include <string>

#define NOMINMAX
#include <Windows.h>

namespace rqdq {
namespace rclw {


struct CLSIDSerializer {
	static CLSID deserialize(const std::wstring&);
};


}  // close package namespace
}  // close enterprise namespace
