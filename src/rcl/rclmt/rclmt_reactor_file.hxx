#pragma once
#include "src/rcl/rclmt/rclmt_deferred.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"

#include <vector>
#include <string>

namespace rqdq {
namespace rclmt {

using LoadFileDeferred = rclmt::Deferred<std::vector<uint8_t>&, uint32_t>;

LoadFileDeferred& LoadFile(const std::string& path, Reactor* reactor=nullptr);
LoadFileDeferred& LoadFile(const std::wstring& path, Reactor* reactor=nullptr);

}  // namespace rclmt
}  // namespace rqdq
