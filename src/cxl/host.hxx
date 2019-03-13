#pragma once
#include <string>

#include "src/cxl/unit.hxx"
#include "src/ral/ralio/ralio_asio.hxx"
#include "src/cxl/log.hxx"

#include <wink/signal.hpp>

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
	bool SetDriver(const std::string& name);
	void SetChannel(const std::string& driverName, int num, const std::string& name);

	void Start();
	void Stop();
	void Restart();

public:
	std::string GetRunningDriverName() { return d_driverName; }
	int GetRunningSampleRate() { return static_cast<int>(d_sampleRate); }
	int GetRunningBufferSize() { return d_bufPreferredSize; }

private:
	void LinkChannel(int ci, const std::string& name);

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
	CXLUnit& d_unit;

	std::string d_wantedDriver;
	std::unordered_map<std::string, std::array<std::string, 2>> d_wantedChannels;

	// channels
	int d_numInputChannels{0};
	int d_numOutputChannels{0};

    std::string d_driverName{"offline"};

	// buffers
	int d_bufMinSize;
	int d_bufMaxSize;
	int d_bufPreferredSize;
	int d_bufGranularity;

	State d_state{State::Stopped};

	// samplerate
	double d_sampleRate;

	// output ready
	bool d_supportsOutputReadyOptimization;

	// buffers and channels
	std::vector<ralio::ASIOBufferInfo> d_bufferInfos;
	std::vector<ralio::ASIOChannelInfo> d_channelInfos;

	std::vector<float> d_dl, d_dr;
	int d_leftIdx{-1};
	int d_rightIdx{-1};

public:
    wink::signal<std::function<void()>> d_updated; };


}  // namespace cxl
}  // namespace rqdq
