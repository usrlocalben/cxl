#pragma once
#include "src/rcl/rclmt/rclmt_reactor.hxx"

#include <functional>

namespace rqdq {
namespace rclmt {

int Delay(double millis, std::function<void()> func, Reactor* reactor=nullptr);
void CancelDelay(int id, Reactor* reactor=nullptr);

}  // namespace rclmt
}  // namespace rqdq
