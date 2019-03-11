#pragma once
#include <string>

#include "src/cxl/unit.hxx"
#include "src/ral/ralio/ralio_asio.hxx"
#include "src/cxl/log.hxx"

namespace rqdq {
namespace cxl {

class CXLASIOHost {
public:
	CXLASIOHost() = default;

	// not copyable or movable
	CXLASIOHost& operator=(const CXLASIOHost&) = delete;
	CXLASIOHost& operator=(CXLASIOHost&&) = delete;
	CXLASIOHost(const CXLASIOHost&) = delete;
	CXLASIOHost(CXLASIOHost&&) = delete;

public:
	void AttachUnit(CXLUnit& unit);
	bool AttachDriver(const std::string& name);
	bool AttachChannel(int num, const std::string& name);
	bool AttachChannel(int num, int idx);

	void Start();
	void Stop();

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
	CXLUnit* d_unit{nullptr};

	// channels
	int d_numInputChannels{0};
	int d_numOutputChannels{0};

	// buffers
	int d_bufMinSize;
	int d_bufMaxSize;
	int d_bufPreferredSize;
	int d_bufGranularity;

	// samplerate
	double d_sampleRate;

	// output ready
	bool d_supportsOutputReadyOptimization;

	// buffers and channels
	std::vector<ralio::ASIOBufferInfo> d_bufferInfos;
	std::vector<ralio::ASIOChannelInfo> d_channelInfos;

	std::vector<float> d_dl, d_dr;
	int d_leftIdx{-1};
	int d_rightIdx{-1}; };


}  // namespace cxl
}  // namespace rqdq
