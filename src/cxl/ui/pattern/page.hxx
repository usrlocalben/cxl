#pragma once
#include <string>

#include "src/cxl/unit.hxx"

namespace rqdq {
namespace cxl {

const std::string GetPageParameterName(const CXLUnit& unit, int pageNum, int trackNum, int paramNum);
int GetPageParameterValue(const CXLUnit& unit, int pageNum, int trackNum, int paramNum);
void AdjustPageParameter(CXLUnit& unit, int pageNum, int trackNum, int paramNum, int offset);


}  // namespace cxl
}  // namespace rqdq
