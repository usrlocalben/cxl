#pragma once
#include <string_view>

#include "src/cxl/unit.hxx"

namespace rqdq {
namespace cxl {

std::string_view GetPageParameterName(const CXLUnit& unit, int pageNum, int trackNum, int paramNum);
int GetPageParameterValue(const CXLUnit& unit, int pageNum, int trackNum, int paramNum);
void AdjustPageParameter(CXLUnit& unit, int pageNum, int trackNum, int paramNum, int offset);


}  // namespace cxl
}  // namespace rqdq
