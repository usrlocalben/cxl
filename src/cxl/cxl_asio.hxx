#pragma once

#include "3rdparty/asiosdk/common/asiosys.h"
#include "3rdparty/asiosdk/common/asio.h"
#include "3rdparty/asiosdk/host/asiodrivers.h"

namespace rqdq {
namespace cxl {

const int kMaxInputChannels = 32;
const int kMaxOutputChannels = 32;

struct DriverInfo {

	// ASIOInit()
	ASIODriverInfo driverInfo;

	// ASIOGetChannels()
	long inputChannels;
	long outputChannels;

	// ASIOGetBufferSize()
	long minSize;
	long maxSize;
	long preferredSize;
	long granularity;

	// ASIOGetSampleRate()
	ASIOSampleRate sampleRate;

	// ASIOOutputReady()
	bool postOutput;

	// ASIOGetLatencies ()
	long inputLatency;
	long outputLatency;

	// ASIOCreateBuffers ()
	long inputBuffers;	// becomes number of actual created input buffers
	long outputBuffers;	// becomes number of actual created output buffers
	ASIOBufferInfo bufferInfos[kMaxInputChannels + kMaxOutputChannels]; // buffer info's

	// ASIOGetChannelInfo()
	ASIOChannelInfo channelInfos[kMaxInputChannels + kMaxOutputChannels]; // channel info's
	// The above two arrays share the same indexing, as the data in them are linked together

	// Information from ASIOGetSamplePosition()
	// data is converted to double floats for easier use, however 64 bit integer can be used, too
	double nanoSeconds;
	double samples;
	double tcSamples;	// time code samples

	// bufferSwitchTimeInfo()
	ASIOTime tInfo;			// time info state
	unsigned long  sysRefTime;      // system reference time, when bufferSwitch() was called

	// Signal the end of processing in this example
	bool stopped; };

}  // close package namespace
}  // close enterprise namespace


extern rqdq::cxl::DriverInfo asioDriverInfo;
extern AsioDrivers* asioDrivers;
//extern ASIOCallbacks asioCallbacks;


// functions
bool loadAsioDriver(const char *name);
long init_asio_static_data(rqdq::cxl::DriverInfo *asioDriverInfo);
//ASIOError create_asio_buffers (DriverInfo *asioDriverInfo);
//unsigned long get_sys_reference_time();

// callback prototypes
//void bufferSwitch(long index, ASIOBool processNow);
//ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);
//void sampleRateChanged(ASIOSampleRate sRate);
//long asioMessages(long selector, long value, void* message, double* opt);
