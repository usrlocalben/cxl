#pragma once
#include <string>

namespace rqdq {
namespace cxl {

namespace config {

extern std::string asioDriverName;
extern std::string masterLeftDest;
extern std::string masterRightDest;
extern std::string dataDir;

void Load();

}

}  // namespace cxl
}  // namespace rqdq
