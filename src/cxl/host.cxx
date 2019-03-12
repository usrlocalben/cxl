#include "src/cxl/host.hxx"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "src/cxl/log.hxx"
#include "src/cxl/unit.hxx"
#include "src/ral/ralio/ralio_asio.hxx"

#include <fmt/printf.h>
#include <wink/signal.hpp>

namespace rqdq {
namespace cxl {

using namespace std;


ralio::ASIOTime* CXLASIOHost::onBufferReadyEx(ralio::ASIOTime* timeInfo, long index, ralio::ASIOBool processNow) {
	using namespace rqdq::ralio;
	static long processedSamples = 0;

	long buffSize = d_bufPreferredSize;  // buffer size in samples
	d_dl.resize(buffSize);
	d_dr.resize(buffSize);
	if (d_unit != nullptr) {
		d_unit->Render(d_dl.data(), d_dr.data(), buffSize); }

	// perform the processing
	for (int i=0; i<d_numInputChannels+d_numOutputChannels; i++) {

		if (!static_cast<bool>(d_bufferInfos[i].isInput)) {
			float* srcPtr = nullptr;
			if (i == d_leftIdx) {
				srcPtr = d_dl.data(); }
			else if (i == d_rightIdx) {
				srcPtr = d_dr.data(); }
			else {
				continue; }

			//cout << "CT(" << int(d_channelInfos[i].type) << ")";
			// OK do processing for the outputs only
			switch (d_channelInfos[i].type) {
			case ASIOSTInt16LSB: {
				auto dst = static_cast<int16_t*>(d_bufferInfos[i].buffers[index]);
				for (int si=0; si<buffSize; si++) {
					dst[si] = static_cast<int16_t>(srcPtr[si] * std::numeric_limits<int16_t>::max());}}
				break;
			case ASIOSTInt24LSB:		// used for 20 bits as well
				memset (d_bufferInfos[i].buffers[index], 0, buffSize * 3);
				break;
			case ASIOSTInt32LSB: {
				auto dst = static_cast<int32_t*>(d_bufferInfos[i].buffers[index]);
				for (int si=0; si<buffSize; si++) {
					dst[si] = static_cast<int32_t>(srcPtr[si] * std::numeric_limits<int32_t>::max());}}
				break;
			case ASIOSTFloat32LSB: {
				// IEEE 754 32 bit float, as found on Intel x86 architecture
				auto dst = static_cast<float*>(d_bufferInfos[i].buffers[index]);
				for (int si=0; si<buffSize; si++) {
					dst[si] = srcPtr[si];}}
				break;
			case ASIOSTFloat64LSB: {
				// IEEE 754 64 bit double float, as found on Intel x86 architecture
				auto dst = static_cast<double*>(d_bufferInfos[i].buffers[index]);
				for (int si=0; si<buffSize; si++) {
					dst[si] = srcPtr[si];}}
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
			case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
				memset (d_bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;

			case ASIOSTInt16MSB:
				memset (d_bufferInfos[i].buffers[index], 0, buffSize * 2);
				break;
			case ASIOSTInt24MSB:		// used for 20 bits as well
				memset (d_bufferInfos[i].buffers[index], 0, buffSize * 3);
				break;
			case ASIOSTInt32MSB:
				memset (d_bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				memset (d_bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				memset (d_bufferInfos[i].buffers[index], 0, buffSize * 8);
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
			case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
				memset (d_bufferInfos[i].buffers[index], 0, buffSize * 4);
				break; }}}

	// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	//if (d_supportsOutputReadyOptimization) {
	//	asio.SignalOutputReady(); }

	processedSamples += buffSize;

	return nullptr; }


long CXLASIOHost::onASIOMessage(long selector, long value, void* message, double* opt) {
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

void CXLASIOHost::AttachUnit(CXLUnit& unit) {
	if (d_unit != nullptr) {
		throw std::runtime_error("CXLASIOHost already has an "
								 "attached CXLUnit instance."); }
	d_unit = &unit; }


bool CXLASIOHost::AttachDriver(const std::string& driverName) {
	auto& asio = ralio::ASIOSystem::GetInstance();
	auto& log = Log::GetInstance();

	asio.RefreshDriverList();

	//std::for_each(begin(asio.d_drivers), end(asio.d_drivers),
	//              [](auto& item) { wcout << item.stringify() << "\n"; });

	int idx = asio.FindDriverByName(driverName);
	if (idx == -1) {
		auto msg = fmt::sprintf("ASIO driver \"%s\" not found in ASIO registry "
		                        "(HKEY_LOCAL_MACHINE\\Software\\ASIO)",
		                        driverName);
		log.info(msg);
		return false; }

	asio.OpenDriver(idx);
	const auto info = asio.Init(nullptr);

	cout << "Driver Initialized:\n";
	cout << "    asioVersion: " << info.asioVersion << "\n";
	cout << "  driverVersion: " << info.driverVersion << "\n";
	cout << "           name: \"" << info.name << "\"" << "\n";
	cout << endl;

	//asio.ShowControlPanel();

	std::tie(d_numInputChannels, d_numOutputChannels) = asio.GetChannels();
	std::tie(d_bufMinSize, d_bufMaxSize, d_bufPreferredSize, d_bufGranularity) = asio.GetBufferSize();

	asio.SetSampleRate(44100.0);
	d_sampleRate = asio.GetSampleRate();
	if (d_sampleRate < 0.0 || d_sampleRate > 96000.0) {
		// sampleRate may not be set, try setting it...
		asio.SetSampleRate(44100.0);
		d_sampleRate = asio.GetSampleRate(); }

	d_supportsOutputReadyOptimization = asio.SignalOutputReady();

	cout << "  #inputChannels: " << d_numInputChannels << "\n";
	cout << " #outputChannels: " << d_numOutputChannels << "\n";
	cout << "\n";
	cout << "    buffer sizes: ";
	cout << d_bufMinSize;
	cout << "/" << d_bufMaxSize;
	cout << "/" << d_bufPreferredSize;
	cout << " (min/max/preferred)\n";
	cout << " buffer granularity: " << d_bufGranularity << "\n";

	cout << "\n";
	cout << " resolution: " << d_sampleRate << " samples/sec" << "\n";
	cout << "\n";

	auto asioCallbacks = MakeASIOCallbacks();

	d_bufferInfos.resize(d_numInputChannels+d_numOutputChannels);
	d_channelInfos.resize(d_numInputChannels+d_numOutputChannels);
	idx = 0;
	for (int i=0; i<d_numInputChannels; i++) {
		auto& info = d_bufferInfos[idx++];
		info.isInput = ralio::ASIOTrue;
		info.channelNum = i;
		info.buffers[0] = info.buffers[1] = nullptr; }
	for (int i=0; i<d_numOutputChannels; i++) {
		auto& info = d_bufferInfos[idx++];
		info.isInput = ralio::ASIOFalse;
		info.channelNum = i;
		info.buffers[0] = info.buffers[1] = nullptr; }

	asio.CreateBuffers(d_bufferInfos.data(),
	                   d_numInputChannels + d_numOutputChannels,
	                   d_bufPreferredSize,
	                   asioCallbacks);

	for (int i=0; i<d_numInputChannels+d_numOutputChannels; i++) {
		d_channelInfos[i] = asio.GetChannelInfo(d_bufferInfos[i].isInput != 0, d_bufferInfos[i].channelNum); }
	return true; }


bool CXLASIOHost::AttachChannel(const int num, const std::string& name) {
	auto& asio = ralio::ASIOSystem::GetInstance();
	auto& log = Log::GetInstance();

	int foundIdx = -1;
	for (int i=0; i<d_numInputChannels+d_numOutputChannels; i++) {
		const auto& info = d_channelInfos[i];
		if (!info.isInput) {
			if (info.name == name) {
				foundIdx = i; }}}

	if (num == 0) {
		d_leftIdx = foundIdx; }
	else if (num == 1) {
		d_rightIdx = foundIdx; }
	else {
		log.info(fmt::sprintf("refusing to attach host to channel %d", num));
		return false; }

	bool wasConnected = foundIdx > -1;
	return wasConnected; }


bool CXLASIOHost::AttachChannel(const int num, const int idx) {
	auto& asio = ralio::ASIOSystem::GetInstance();
	auto& log = Log::GetInstance();

	int foundIdx = -1;
	for (int i=0; i<d_numInputChannels+d_numOutputChannels; i++) {
		const auto& info = d_channelInfos[i];
		if (!info.isInput) {
			if (info.channel == idx) {
				foundIdx = i; }}}

	if (num == 0) {
		d_leftIdx = foundIdx; }
	else if (num == 1) {
		d_rightIdx = foundIdx; }
	else {
		log.info(fmt::sprintf("refusing to attach host to channel %d", num));
		return false; }

	bool wasConnected = foundIdx > -1;
	return wasConnected; }


void CXLASIOHost::Start() {
	auto& asio = ralio::ASIOSystem::GetInstance();
	asio.Start(); }


void CXLASIOHost::Stop() {
	auto& asio = ralio::ASIOSystem::GetInstance();
	asio.Stop();
	asio.DisposeBuffers();
	asio.Exit(); }


}  // namespace cxl
}  // namespace rqdq