#include "src/cxl/cxl_reactor.hxx"

#include <functional>
#include <vector>

#define NOMINMAX
#include <Windows.h>

namespace rqdq {
namespace cxl {

Reactor& Reactor::GetInstance() {
	static Reactor reactor;
	return reactor; }


void Reactor::Run() {
	std::vector<HANDLE> pendingEvents;
	while (!d_shouldQuit) {
		pendingEvents.clear();
		for (auto& re : d_events) {
			pendingEvents.emplace_back(re.event); }
		DWORD result = WaitForMultipleObjects(pendingEvents.size(), pendingEvents.data(), FALSE, 1000);
		if (result == WAIT_TIMEOUT) {
			// nothing
			}
		else if (WAIT_OBJECT_0 <= result && result < (WAIT_OBJECT_0+pendingEvents.size())) {
			int signaledIdx = result - WAIT_OBJECT_0;
			auto& re = d_events[signaledIdx];
			if (re.func) {
				re.func(); }}}}


}  // namespace cxl
}  // namespace rqdq
