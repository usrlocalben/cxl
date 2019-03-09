#include "src/cxl/config.hxx"
#include "src/cxl/unit.hxx"
#include "src/cxl/log.hxx"
#include "src/cxl/ui/root/view.hxx"
#include "src/ral/ralio/ralio_asio.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rcls/rcls_console.hxx"

#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "3rdparty/fmt/include/fmt/format.h"
#include "3rdparty/fmt/include/fmt/printf.h"
#include "3rdparty/wink/wink/signal.hpp"
#include <Windows.h>

namespace rqdq {

namespace {

// channels
int numInputChannels = 0;
int numOutputChannels = 0;

// buffers
int bufMinSize;
int bufMaxSize;
int bufPreferredSize;
int bufGranularity;

// samplerate
double sampleRate;

// output ready
bool supportsOutputReadyOptimization;

// buffers and channels
std::vector<ralio::ASIOBufferInfo> bufferInfos;
std::vector<ralio::ASIOChannelInfo> channelInfos;

/**
 * based on https://stackoverflow.com/questions/1517685/recursive-createdirectory
 */
void EnsureDirectoryExists(const std::string& path) {
	size_t pos = 0;
	do {
		pos = path.find_first_of("\\/", pos+1);
		CreateDirectoryA(path.substr(0, pos).c_str(), nullptr);
	} while (pos != std::string::npos); }


}  // namespace

namespace cxl {

using namespace std;


class ASIOConnector {
public:
	ASIOConnector(CXLUnit& unit) :d_unit(unit) {}

	ralio::ASIOCallbacks MakeASIOCallbacks() {
		rqdq::ralio::ASIOCallbacks out{
			/*bufferSwitch=*/&ASIOConnector::onBufferReadyJmp,
			/*sampleRateDidChange=*/&ASIOConnector::onSampleRateChangedJmp,
			/*asioMessage=*/&ASIOConnector::onASIOMessageJmp,
			/*bufferSwitchTimeInfo=*/&ASIOConnector::onBufferReadyExJmp,
			/*ptr=*/static_cast<void*>(this) };
		return out; }

	void Connect(int leftIdx, int rightIdx) {
		d_leftIdx = leftIdx;
		d_rightIdx = rightIdx;
		d_connectionChangedSignal.emit(d_leftIdx, d_rightIdx); }

	static ralio::ASIOTime* onBufferReadyExJmp(void* ptr, ralio::ASIOTime *timeInfo, long index, ralio::ASIOBool processNow) {
		auto& self = *static_cast<ASIOConnector*>(ptr);
		return self.onBufferReadyEx(timeInfo, index, processNow); }
	ralio::ASIOTime* onBufferReadyEx(ralio::ASIOTime* timeInfo, long index, ralio::ASIOBool processNow) {
		using namespace rqdq::ralio;
		static long processedSamples = 0;

		long buffSize = bufPreferredSize;  // buffer size in samples
		dl.resize(buffSize);
		dr.resize(buffSize);
		d_unit.Render(dl.data(), dr.data(), buffSize);

		// perform the processing
		for (int i=0; i<numInputChannels+numOutputChannels; i++) {

			if (!static_cast<bool>(bufferInfos[i].isInput)) {
				float* srcPtr = nullptr;
				if (i == d_leftIdx) {
					srcPtr = dl.data(); }
				else if (i == d_rightIdx) {
					srcPtr = dr.data(); }
				else {
					continue; }

				//cout << "CT(" << int(channelInfos[i].type) << ")";
				// OK do processing for the outputs only
				switch (channelInfos[i].type) {
				case ASIOSTInt16LSB: {
					auto dst = static_cast<int16_t*>(bufferInfos[i].buffers[index]);
					for (int si=0; si<buffSize; si++) {
						dst[si] = static_cast<int16_t>(srcPtr[si] * std::numeric_limits<int16_t>::max());}}
					break;
				case ASIOSTInt24LSB:		// used for 20 bits as well
					memset (bufferInfos[i].buffers[index], 0, buffSize * 3);
					break;
				case ASIOSTInt32LSB: {
					auto dst = static_cast<int32_t*>(bufferInfos[i].buffers[index]);
					for (int si=0; si<buffSize; si++) {
						dst[si] = static_cast<int32_t>(srcPtr[si] * std::numeric_limits<int32_t>::max());}}
					break;
				case ASIOSTFloat32LSB: {
					// IEEE 754 32 bit float, as found on Intel x86 architecture
					auto dst = static_cast<float*>(bufferInfos[i].buffers[index]);
					for (int si=0; si<buffSize; si++) {
						dst[si] = srcPtr[si];}}
					break;
				case ASIOSTFloat64LSB: {
					// IEEE 754 64 bit double float, as found on Intel x86 architecture
					auto dst = static_cast<double*>(bufferInfos[i].buffers[index]);
					for (int si=0; si<buffSize; si++) {
						dst[si] = srcPtr[si];}}
					break;

					// these are used for 32 bit data buffer, with different alignment of the data inside
					// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
				case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
					memset (bufferInfos[i].buffers[index], 0, buffSize * 4);
					break;

				case ASIOSTInt16MSB:
					memset (bufferInfos[i].buffers[index], 0, buffSize * 2);
					break;
				case ASIOSTInt24MSB:		// used for 20 bits as well
					memset (bufferInfos[i].buffers[index], 0, buffSize * 3);
					break;
				case ASIOSTInt32MSB:
					memset (bufferInfos[i].buffers[index], 0, buffSize * 4);
					break;
				case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
					memset (bufferInfos[i].buffers[index], 0, buffSize * 4);
					break;
				case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
					memset (bufferInfos[i].buffers[index], 0, buffSize * 8);
					break;

					// these are used for 32 bit data buffer, with different alignment of the data inside
					// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
				case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
					memset (bufferInfos[i].buffers[index], 0, buffSize * 4);
					break; }}}

		// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
		//if (supportsOutputReadyOptimization) {
		//	asio.SignalOutputReady(); }

		processedSamples += buffSize;

		return nullptr; }

	static void onBufferReadyJmp(void* ptr, long index, ralio::ASIOBool processNow) {
		auto& self = *static_cast<ASIOConnector*>(ptr);
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
		auto& self = *static_cast<ASIOConnector*>(ptr);
		return self.onSampleRateChanged(sRate); }
	void onSampleRateChanged(ralio::ASIOSampleRate sRate) {}


	/**
	 * asio message handler callback
	 */
	static long onASIOMessageJmp(void* ptr, long selector, long value, void* message, double* opt) {
		auto& self = *static_cast<ASIOConnector*>(ptr);
		return self.onASIOMessage(selector, value, message, opt); }
	long onASIOMessage(long selector, long value, void* message, double* opt) {
		using namespace ralio;
		// currently the parameters "value", "message" and "opt" are not used.
		long ret = 0;
		switch(selector) {
			case kAsioSelectorSupported:
				if (value == kAsioResetRequest ||
					value == kAsioEngineVersion ||
					value == kAsioResyncRequest ||
					value == kAsioLatenciesChanged ||
				// the following three were added for ASIO 2.0, you don't necessarily have to support them
					value == kAsioSupportsTimeInfo ||
					value == kAsioSupportsTimeCode ||
					value == kAsioSupportsInputMonitor) {
					ret = 1L; }
				break;
			case kAsioResetRequest:
				// defer the task and perform the reset of the driver during the next "safe" situation
				// You cannot reset the driver right now, as this code is called from the driver.
				// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
				// Afterwards you initialize the driver again.
				// XXX unsupported
				ret = 1L;
				break;
			case kAsioResyncRequest:
				// This informs the application, that the driver encountered some non fatal data loss.
				// It is used for synchronization purposes of different media.
				// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
				// Windows Multimedia system, which could loose data because the Mutex was hold too long
				// by another thread.
				// However a driver can issue it in other situations, too.
				ret = 1L;
				break;
			case kAsioLatenciesChanged:
				// This will inform the host application that the drivers were latencies changed.
				// Beware, it this does not mean that the buffer sizes have changed!
				// You might need to update internal delay data.
				ret = 1L;
				break;
			case kAsioEngineVersion:
				// return the supported ASIO version of the host application
				// If a host applications does not implement this selector, ASIO 1.0 is assumed
				// by the driver
				ret = 2L;
				break;
			case kAsioSupportsTimeInfo:
				// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
				// is supported.
				// For compatibility with ASIO 1.0 drivers the host application should always support
				// the "old" bufferSwitch method, too.
				ret = 1;
				break;
			case kAsioSupportsTimeCode:
				// informs the driver wether application is interested in time code info.
				// If an application does not need to know about time code, the driver has less work
				// to do.
				ret = 0;
				break; }
		return ret; }
public:
	wink::signal<wink::slot<void(int, int)>> d_connectionChangedSignal;
private:
	std::vector<float> dl, dr;
	int d_leftIdx = -1;
	int d_rightIdx = -1;
	CXLUnit& d_unit; };


int main(int argc, char **argv) {
	config::Load();
	auto& log = Log::GetInstance();
	log.info("Log started");


	const array<string, 3> userDirs = {config::patternDir, config::sampleDir, config::kitDir};
	for_each(userDirs.begin(), userDirs.end(), EnsureDirectoryExists);

	auto& asio = ralio::ASIOSystem::GetInstance();
	asio.RefreshDriverList();

	std::for_each(begin(asio.d_drivers), end(asio.d_drivers),
				  [](auto& item) { wcout << item.stringify() << "\n"; });

	int idx = asio.FindDriverByName(config::asioDriverName);
	if (idx == -1) {
		auto msg = fmt::format("ASIO driver \"{}\" not found in ASIO registry "
							   "(HKEY_LOCAL_MACHINE\\Software\\ASIO)",
							   config::asioDriverName);
		throw std::runtime_error(msg); }

	asio.OpenDriver(idx);
	const auto info = asio.Init(nullptr);

	cout << "Driver Initialized:\n";
	cout << "    asioVersion: " << info.asioVersion << "\n";
	cout << "  driverVersion: " << info.driverVersion << "\n";
	cout << "           name: \"" << info.name << "\"" << "\n";
	cout << endl;

	//asio.ShowControlPanel();

	std::tie(numInputChannels, numOutputChannels) = asio.GetChannels();
	std::tie(bufMinSize, bufMaxSize, bufPreferredSize, bufGranularity) = asio.GetBufferSize();

	asio.SetSampleRate(44100.0);
	sampleRate = asio.GetSampleRate();
	if (sampleRate < 0.0 || sampleRate > 96000.0) {
		// sampleRate may not be set, try setting it...
		asio.SetSampleRate(44100.0);
		sampleRate = asio.GetSampleRate(); }

	supportsOutputReadyOptimization = asio.SignalOutputReady();

	cout << "  #inputChannels: " << numInputChannels << "\n";
	cout << " #outputChannels: " << numOutputChannels << "\n";
	cout << "\n";
	cout << "    buffer sizes: ";
	cout << bufMinSize;
	cout << "/" << bufMaxSize;
	cout << "/" << bufPreferredSize;
	cout << " (min/max/preferred)\n";
	cout << " buffer granularity: " << bufGranularity << "\n";

	cout << "\n";
	cout << " resolution: " << sampleRate << " samples/sec" << "\n";
	cout << "\n";

	CXLUnit unit;
	ASIOConnector connector{ unit };
	auto asioCallbacks = connector.MakeASIOCallbacks();

	connector.d_connectionChangedSignal.connect([](int a, int b) { cout << "connection changed to " << a << ", " << b << "\n"; });


	bufferInfos.resize(numInputChannels+numOutputChannels);
	channelInfos.resize(numInputChannels+numOutputChannels);
	idx = 0;
	for (int i=0; i<numInputChannels; i++) {
		auto& info = bufferInfos[idx++];
		info.isInput = ralio::ASIOTrue;
		info.channelNum = i;
		info.buffers[0] = info.buffers[1] = nullptr; }
	for (int i=0; i<numOutputChannels; i++) {
		auto& info = bufferInfos[idx++];
		info.isInput = ralio::ASIOFalse;
		info.channelNum = i;
		info.buffers[0] = info.buffers[1] = nullptr; }

	asio.CreateBuffers(bufferInfos.data(),
	                   numInputChannels + numOutputChannels,
	                   bufPreferredSize,
	                   &asioCallbacks);

	int leftChannelIdx = -1;
	int rightChannelIdx = -1;
	for (int i=0; i<numInputChannels+numOutputChannels; i++) {
		channelInfos[i] = asio.GetChannelInfo(bufferInfos[i].isInput != 0, bufferInfos[i].channelNum);
		const auto& info = channelInfos[i];

		if (!info.isInput) {
			string mlc = config::masterLeftDest;
			if (rclt::ConsumePrefix(mlc, "name=")) {
				// identify connection by channel name
				if (info.name==mlc) {
					leftChannelIdx = i; }}
			else if (rclt::ConsumePrefix(mlc, "num=")) {
				// identify connection by index
				if (info.channel==stoi(mlc)) {
					leftChannelIdx = i; }}
			else {
				auto msg = fmt::format("invalid asio connection \"{}\" expected "
				                       "either num=<num> or name=<text>", config::masterLeftDest);
				throw std::runtime_error(msg); }

			mlc = config::masterRightDest;
			if (rclt::ConsumePrefix(mlc, "name=")) {
				// identify connection by channel name
				if (info.name==mlc) {
					rightChannelIdx = i; }}
			else if (rclt::ConsumePrefix(mlc, "num=")) {
				// identify connection by index
				if (info.channel==stoi(mlc)) {
					rightChannelIdx = i; }}
			else {
				auto msg = fmt::format("invalid asio connection \"{}\" expected "
				                       "either num=<num> or name=<text>", config::masterRightDest);
				throw std::runtime_error(msg); }}

		auto detail = fmt::format("Ch #{}, {} \"{}\" {}",
		                          info.channel, info.isInput?" INPUT":"OUTPUT",
		                          info.name, info.type);
		cout << detail << endl; }

	if (leftChannelIdx == -1 || rightChannelIdx == -1) {
		cerr << "one or more master output channels were not connected to ASIO channels.  aborting.\n";
		return 1; }

	connector.Connect(leftChannelIdx, rightChannelIdx);

	cout << "Starting in ";
	for (int i=3; i>=1; i--) {
		cout << i << "...";
		Sleep(1000); }
	cout << "\n";

	asio.Start();

	auto& console = rcls::Console::GetInstance();
	console.SetDimensions(80, 25);
	console.Clear();

	UIRoot(unit).Run();

	asio.Stop();
	asio.DisposeBuffers();
	asio.Exit();
	cout << "==== END ====\n" << flush;
	return 0; }


}  // namespace cxl
}  // namespace rqdq


int main(int argc, char **argv) {
	int result;
	try {
		result = rqdq::cxl::main(argc, argv); }
	catch (const std::exception& err) {
		std::cerr << "exception: " << err.what() << std::endl;
		result = EXIT_FAILURE; }
	catch (...) {
		std::cerr << "unknown exception" << std::endl;
		result = EXIT_FAILURE; }
	return result; }


