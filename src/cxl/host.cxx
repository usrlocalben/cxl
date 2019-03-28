#include "host.hxx"

#include <algorithm>
#include <array>
#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "src/cxl/log.hxx"
#include "src/cxl/unit.hxx"
#include "src/ral/ralio/ralio_asio.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

using namespace std;

CXLASIOHost::CXLASIOHost(CXLUnit& unit) :unit_(unit) {}


ralio::ASIOTime* CXLASIOHost::onBufferReadyEx(ralio::ASIOTime* timeInfo, long index, ralio::ASIOBool processNow) {
	using namespace rqdq::ralio;
	static long processedSamples = 0;

	long buffSize = bufPreferredSize_*2;  // buffer size in samples
	dl_.resize(buffSize);
	dr_.resize(buffSize);
	unit_.Render(dl_.data(), dr_.data(), buffSize);

	// perform the processing
	for (int i=0; i<numInputChannels_+numOutputChannels_; i++) {

		if (!static_cast<bool>(bufferInfos_[i].isInput)) {
			float* srcPtr = nullptr;
			if (i == leftIdx_) {
				srcPtr = dl_.data(); }
			else if (i == rightIdx_) {
				srcPtr = dr_.data(); }
			else {
				continue; }

			switch (channelInfos_[i].type) {
			case ASIOSTInt16LSB: {
				auto dst = static_cast<int16_t*>(bufferInfos_[i].buffers[index]);
				for (int si=0; si<buffSize; si++) {
					dst[si] = static_cast<int16_t>(srcPtr[si] * std::numeric_limits<int16_t>::max());}}
				break;
			// case ASIOSTInt24LSB:		// used for 20 bits as well
			//	memset (bufferInfos_[i].buffers[index], 0, buffSize * 3);
			//	break;
			case ASIOSTInt32LSB: {
				auto dst = static_cast<int32_t*>(bufferInfos_[i].buffers[index]);
				for (int si=0; si<buffSize; si++) {
					dst[si] = static_cast<int32_t>(srcPtr[si] * std::numeric_limits<int32_t>::max());}}
				break;
			case ASIOSTFloat32LSB: {
				// IEEE 754 32 bit float, as found on Intel x86 architecture
				auto dst = static_cast<float*>(bufferInfos_[i].buffers[index]);
				for (int si=0; si<buffSize; si++) {
					dst[si] = srcPtr[si];}}
				break;
			case ASIOSTFloat64LSB: {
				// IEEE 754 64 bit double float, as found on Intel x86 architecture
				auto dst = static_cast<double*>(bufferInfos_[i].buffers[index]);
				for (int si=0; si<buffSize; si++) {
					dst[si] = srcPtr[si];}}
				break;

/*
				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
			case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
				memset (bufferInfos_[i].buffers[index], 0, buffSize * 4);
				break;

			case ASIOSTInt16MSB:
				memset (bufferInfos_[i].buffers[index], 0, buffSize * 2);
				break;
			case ASIOSTInt24MSB:		// used for 20 bits as well
				memset (bufferInfos_[i].buffers[index], 0, buffSize * 3);
				break;
			case ASIOSTInt32MSB:
				memset (bufferInfos_[i].buffers[index], 0, buffSize * 4);
				break;
			case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				memset (bufferInfos_[i].buffers[index], 0, buffSize * 4);
				break;
			case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				memset (bufferInfos_[i].buffers[index], 0, buffSize * 8);
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
			case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
				memset (bufferInfos_[i].buffers[index], 0, buffSize * 4);
				break;
*/
			default:
				// wtf = channelInfos_[i].type;
				break; }}}

	// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	//if (supportsOutputReadyOptimization_) {
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


bool CXLASIOHost::SetDriver(std::string_view name) {
	auto& asio = ralio::ASIOSystem::GetInstance();
	auto& log = Log::GetInstance();

	asio.RefreshDriverList();

	int idx = asio.FindDriver(name);
	if (idx == -1) {
		auto msg = fmt::sprintf("ASIO driver \"%s\" not found in ASIO registry "
		                        "(HKEY_LOCAL_MACHINE\\Software\\ASIO)",
		                        name);
		log.info(msg);
		return false; }

	wantedDriver_ = name;
	if (state_ == State::Running) {
		Restart(); }
	return true; }


void CXLASIOHost::SetChannel(const std::string& driverName, int num, std::string_view name) {
	wantedChannels_[driverName][num] = name;
	if (driverName == driverName_) {
		Restart(); }}


void CXLASIOHost::Start() {
	auto& asio = ralio::ASIOSystem::GetInstance();
	auto& log = Log::GetInstance();
	if (state_ == State::Running) {
		return; }

	int idx = asio.FindDriver(wantedDriver_);
	assert(idx > -1);

	try {
		asio.OpenDriver(idx); }
	catch (const std::exception& err) {
		driverName_ = "error";
		auto msg = fmt::sprintf("ASIO OpenDriver error: %s", err.what());
		log.info(msg);
		updated_.Emit();
		state_ = State::Failed;
		return; }

    driverName_ = wantedDriver_;

	const auto info = asio.Init(nullptr);

	/*
	cout << "Driver Initialized:\n";
	cout << "    asioVersion: " << info.asioVersion << "\n";
	cout << "  driverVersion: " << info.driverVersion << "\n";
	cout << "           name: \"" << info.name << "\"" << "\n";
	cout << endl;
	*/

	//asio.ShowControlPanel();

	std::tie(numInputChannels_, numOutputChannels_) = asio.GetChannels();
	std::tie(bufMinSize_, bufMaxSize_, bufPreferredSize_, bufGranularity_) = asio.GetBufferSize();

	asio.SetSampleRate(44100.0);
	sampleRate_ = asio.GetSampleRate();
	if (sampleRate_ < 0.0 || sampleRate_ > 96000.0) {
		// sampleRate may not be set, try setting it...
		asio.SetSampleRate(44100.0);
		sampleRate_ = asio.GetSampleRate(); }

	supportsOutputReadyOptimization_ = asio.SignalOutputReady();

	bufferInfos_.resize(numInputChannels_+numOutputChannels_);
	channelInfos_.resize(numInputChannels_+numOutputChannels_);
	idx = 0;
	for (int i=0; i<numInputChannels_; i++) {
		auto& info = bufferInfos_[idx++];
		info.isInput = ralio::ASIOTrue;
		info.channelNum = i;
		info.buffers[0] = info.buffers[1] = nullptr; }
	for (int i=0; i<numOutputChannels_; i++) {
		auto& info = bufferInfos_[idx++];
		info.isInput = ralio::ASIOFalse;
		info.channelNum = i;
		info.buffers[0] = info.buffers[1] = nullptr; }

	asio.CreateBuffers(bufferInfos_.data(),
	                   numInputChannels_ + numOutputChannels_,
	                   bufPreferredSize_*2,
	                   MakeASIOCallbacks());

	for (int i=0; i<numInputChannels_+numOutputChannels_; i++) {
		channelInfos_[i] = asio.GetChannelInfo(bufferInfos_[i].isInput != 0, bufferInfos_[i].channelNum); }

	rightIdx_ = -1;
	for (int ci=0; ci<2; ci++) {
		auto& name = wantedChannels_[driverName_][ci];
		LinkChannel(ci, name); }

	asio.Start();
	state_ = State::Running;
	log.info("host: Running");
	updated_.Emit(); }


void CXLASIOHost::LinkChannel(const int num, std::string_view name) {
	auto& log = Log::GetInstance();

	int foundIdx = -1;
	for (int i=0; i<numInputChannels_+numOutputChannels_; i++) {
		const auto& info = channelInfos_[i];
		if (!info.isInput) {
			if (info.name == name) {
				foundIdx = i; }}}

	if (num == 0) {
		log.info(fmt::sprintf("linked left output to %d", foundIdx));
		leftIdx_ = foundIdx; }
	else if (num == 1) {
		log.info(fmt::sprintf("linked right output to %d", foundIdx));
		rightIdx_ = foundIdx; }
	else {
		log.info(fmt::sprintf("refusing to attach host to channel %d", num)); }}


void CXLASIOHost::Stop() {
	if (state_ == State::Stopped) {
		return; }
	auto& asio = ralio::ASIOSystem::GetInstance();
	asio.Stop();
	asio.DisposeBuffers();
	state_ = State::Stopped;
	auto& log = Log::GetInstance();
	log.info("host: Stopped");
	updated_.Emit(); }


void CXLASIOHost::Restart() {
	Stop();
	Start(); }


CXLASIOHost::~CXLASIOHost() {
	auto& asio = ralio::ASIOSystem::GetInstance();
	if (state_ == State::Running) {
		Stop(); }
	asio.Exit(); }


}  // namespace cxl
}  // namespace rqdq
