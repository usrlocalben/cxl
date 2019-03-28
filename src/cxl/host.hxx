#pragma once
#include <array>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/cxl/unit.hxx"
#include "src/ral/ralio/ralio_asio.hxx"
#include "src/rcl/rclmt/rclmt_signal.hxx"
#include "src/cxl/log.hxx"

namespace rqdq {
namespace cxl {

class CXLASIOHost {
public:
	CXLASIOHost(CXLUnit& unit);

	// not copyable or movable
	CXLASIOHost& operator=(const CXLASIOHost&) = delete;
	CXLASIOHost& operator=(CXLASIOHost&&) = delete;
	CXLASIOHost(const CXLASIOHost&) = delete;
	CXLASIOHost(CXLASIOHost&&) = delete;

	~CXLASIOHost();

	enum class State {
		Stopped,
		Running,
		Failed, };

public:
	bool SetDriver(std::string_view name);
	void SetChannel(const std::string& driverName, int num, std::string_view name);

	void Start();
	void Stop();
	void Restart();

public:
	std::string_view GetRunningDriverName() const { return driverName_; }
	int GetRunningSampleRate() const { return static_cast<int>(sampleRate_); }
	int GetRunningBufferSize() const { return bufPreferredSize_; }

private:
	void LinkChannel(int num, std::string_view name);

	ralio::ASIOCallbacks* MakeASIOCallbacks() {
		static rqdq::ralio::ASIOCallbacks out{
			/*bufferSwitch=*/&CXLASIOHost::onBufferReadyJmp,
			/*sampleRateDidChange=*/&CXLASIOHost::onSampleRateChangedJmp,
			/*asioMessage=*/&CXLASIOHost::onASIOMessageJmp,
			/*bufferSwitchTimeInfo=*/&CXLASIOHost::onBufferReadyExJmp,
			/*ptr=*/static_cast<void*>(this) };
		return &out; }

	static ralio::ASIOTime* onBufferReadyExJmp(void* ptr, ralio::ASIOTime *timeInfo, long index, ralio::ASIOBool processNow) {
		auto& self = *static_cast<CXLASIOHost*>(ptr);
		return self.onBufferReadyEx(timeInfo, index, processNow); }
	ralio::ASIOTime* onBufferReadyEx(ralio::ASIOTime* timeInfo, long index, ralio::ASIOBool processNow);

	static void onBufferReadyJmp(void* ptr, long index, ralio::ASIOBool processNow) {
		auto& self = *static_cast<CXLASIOHost*>(ptr);
		return self.onBufferReady(index, processNow); }
	void onBufferReady(long index, ralio::ASIOBool processNow) {
		using namespace ralio;
		ASIOTime timeInfo{};
		memset(&timeInfo, 0, sizeof (timeInfo));

		// get the time stamp of the buffer, not necessary if no
		// synchronization to other media is required
		//if (asio.OGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		//	timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

		onBufferReadyEx(&timeInfo, index, processNow); }

	static void onSampleRateChangedJmp(void* ptr, ralio::ASIOSampleRate sRate) {
		auto& self = *static_cast<CXLASIOHost*>(ptr);
		return self.onSampleRateChanged(sRate); }
	void onSampleRateChanged(ralio::ASIOSampleRate sRate) {}


	static long onASIOMessageJmp(void* ptr, long selector, long value, void* message, double* opt) {
		auto& self = *static_cast<CXLASIOHost*>(ptr);
		return self.onASIOMessage(selector, value, message, opt); }
	long onASIOMessage(long selector, long value, void* message, double* opt);

private:
	CXLUnit& unit_;

	std::string wantedDriver_;
	std::unordered_map<std::string, std::array<std::string, 2>> wantedChannels_;

	// channels
	int numInputChannels_{0};
	int numOutputChannels_{0};

    std::string driverName_{"offline"};

	// buffers
	int bufMinSize_;
	int bufMaxSize_;
	int bufPreferredSize_;
	int bufGranularity_;

	State state_{State::Stopped};

	// samplerate
	double sampleRate_;

	// output ready
	bool supportsOutputReadyOptimization_;

	// buffers and channels
	std::vector<ralio::ASIOBufferInfo> bufferInfos_;
	std::vector<ralio::ASIOChannelInfo> channelInfos_;

	std::vector<float> dl_, dr_;
	int leftIdx_{-1};
	int rightIdx_{-1};

public:
    rclmt::Signal<void()> updated_; };


}  // namespace cxl
}  // namespace rqdq
