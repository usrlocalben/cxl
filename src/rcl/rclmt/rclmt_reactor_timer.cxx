#include "src/rcl/rclmt/rclmt_reactor_timer.hxx"

#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"

#include <fmt/printf.h>
#include <Windows.h>

namespace rqdq {
namespace {

constexpr bool TT_ONESHOT{false};
constexpr bool TT_PERIODIC{true};

// timers
// todo: since reactor* could be different, it should be stored
//       in the TimerInfo so CancelTimer can reuse it
struct TimerInfo {
	bool inUse{false};
	bool canceled{false};
	int id{0};
	bool type{TT_ONESHOT};
	rclmt::Event event{rclmt::Event::MakeTimer()}; };

std::vector<TimerInfo> timers;

int timerSeq = 1;

std::pair<int, rclmt::Event*> AllocTimer(bool type) {
	TimerInfo* timer{nullptr};

	auto available = std::find_if(timers.begin(), timers.end(),
	                              [](auto& item) { return !item.inUse; });
	if (available != timers.end()) {
		timer = &(*available);}
	else {
		timers.emplace_back();
		timer = &timers.back();}

	timer->inUse = true;
	timer->canceled = false;
	timer->type = type;
	timer->id = timerSeq++;
	return {timer->id, &timers.back().event}; }


bool MaybeReleaseTimer(HANDLE h) {
	auto found = std::find_if(timers.begin(), timers.end(),
	                          [=](auto& item) { return item.event.Get() == h; });
	if (found == timers.end()) {
		throw std::runtime_error("MaybeReleaseTimer called with handle not found in timer collection"); }

	auto& timer = *found;
	bool canceled = timer.canceled;
	if (canceled || timer.type == TT_ONESHOT) {
		timer.inUse = false;
		timer.id = 0;
		timer.canceled = false; }
	return canceled; }


}  // namespace

namespace rclmt {

/**
 * Delay()
 * similar to Twisted callLater() or JavaScript's setTimeout()
 */
int Delay(const double millis, std::function<void()> func, Reactor* reactor_/*=nullptr*/) {
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();

	auto [id, timer] = AllocTimer(TT_ONESHOT);
	timer->SignalIn(millis);
	auto rawHandle = timer->Get();
	reactor.ListenOnce(*timer, [func{std::move(func)}, rawHandle](){
		bool canceled = MaybeReleaseTimer(rawHandle);
		if (!canceled) {
			func(); } });
	return id; };


int Repeat(const double millis, std::function<void()> func, Reactor* reactor_/*=nullptr*/) {
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();

	auto [id, timer] = AllocTimer(TT_PERIODIC);
	timer->SignalEvery(millis);
	auto rawHandle = timer->Get();
	reactor.ListenMany(*timer, [func{std::move(func)}, rawHandle, &reactor](){
		bool canceled = MaybeReleaseTimer(rawHandle);
		if (!canceled) {
			func(); }
		else {
			reactor.RemoveEventByHandle(rawHandle); } });
	return id; };


void CancelTimer(int id, Reactor* reactor_/*=nullptr*/) {
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();

	auto found = std::find_if(timers.begin(), timers.end(),
	                          [=](auto& item) { return item.id = id; });
	if (found == timers.end()) {
		return; }  // not found is a no-op

	found->canceled = true;
	auto handle = found->event.Get();
	auto result = CancelWaitableTimer(handle);
	if (result != 0) {
		auto error = GetLastError();
		auto msg = fmt::sprintf("CancelWaitableTimer error %d", error);
		throw std::runtime_error(msg); }
	reactor.RemoveEventByHandle(handle);
	found->inUse = false; }


void CancelDelay(int id, Reactor* reactor_/*=nullptr*/) {
	return CancelTimer(id, reactor_); }


void CancelRepeat(int id, Reactor* reactor_/*=nullptr*/) {
	return CancelTimer(id, reactor_); }


}  // namespace rclmt
}  // namespace rqdq
