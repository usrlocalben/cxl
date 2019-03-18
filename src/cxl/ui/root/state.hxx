#pragma once

#include <array>

namespace rqdq {
namespace cxl {

constexpr int UM_PATTERN = 0;
constexpr int UM_LOG = 1;
constexpr int UM_HOST = 2;

extern const std::array<int, 3> UI_MODES;


}  // namespace cxl
}  // namespace rqdq
