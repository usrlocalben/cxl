#pragma once
#include <string>

namespace rqdq {
namespace cxl {

namespace config {

extern std::string asioDriverName;
extern std::string masterLeftDest;
extern std::string masterRightDest;
extern std::string dataDir;
extern std::string sampleDir;
extern std::string patternDir;
extern std::string kitDir;

void Load();

}  // namespace config


}  // namespace cxl
}  // namespace rqdq
