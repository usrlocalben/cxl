#pragma once

#include <functional>
#include <vector>

#define NOMINMAX
#include <Windows.h>

namespace rqdq {
namespace cxl {

struct ReactorEvent {
	HANDLE event;
	std::function<void()> func; };


class Reactor {
private:
	Reactor() = default;
public:
	static Reactor& GetInstance();

	void Run();

	void Stop() {
		d_shouldQuit = true; }

	void AddEvent(ReactorEvent re) {
		d_events.emplace_back(re); }

private:
	bool d_shouldQuit = false;
	std::vector<ReactorEvent> d_events; };


}  // namespace cxl
}  // namespace rqdq
