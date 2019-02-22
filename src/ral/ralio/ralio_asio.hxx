#pragma once
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace rqdq {
namespace ralio {

using ASIOSamples = long long;
using ASIOTimeStamp = long long;
using ASIOSampleRate = double;
using ASIOBool = long;
enum {
	ASIOFalse = 0,
	ASIOTrue = 1 };

using ASIOSampleType = long;

enum {
	ASIOSTInt16MSB   = 0,
	ASIOSTInt24MSB   = 1,		// used for 20 bits as well
	ASIOSTInt32MSB   = 2,
	ASIOSTFloat32MSB = 3,		// IEEE 754 32 bit float
	ASIOSTFloat64MSB = 4,		// IEEE 754 64 bit double float

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can be more easily used with these
	ASIOSTInt32MSB16 = 8,		// 32 bit data with 16 bit alignment
	ASIOSTInt32MSB18 = 9,		// 32 bit data with 18 bit alignment
	ASIOSTInt32MSB20 = 10,		// 32 bit data with 20 bit alignment
	ASIOSTInt32MSB24 = 11,		// 32 bit data with 24 bit alignment

	ASIOSTInt16LSB   = 16,
	ASIOSTInt24LSB   = 17,		// used for 20 bits as well
	ASIOSTInt32LSB   = 18,
	ASIOSTFloat32LSB = 19,		// IEEE 754 32 bit float, as found on Intel x86 architecture
	ASIOSTFloat64LSB = 20, 		// IEEE 754 64 bit double float, as found on Intel x86 architecture

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	ASIOSTInt32LSB16 = 24,		// 32 bit data with 18 bit alignment
	ASIOSTInt32LSB18 = 25,		// 32 bit data with 18 bit alignment
	ASIOSTInt32LSB20 = 26,		// 32 bit data with 20 bit alignment
	ASIOSTInt32LSB24 = 27,		// 32 bit data with 24 bit alignment

	//	ASIO DSD format.
	ASIOSTDSDInt8LSB1   = 32,		// DSD 1 bit data, 8 samples per byte. First sample in Least significant bit.
	ASIOSTDSDInt8MSB1   = 33,		// DSD 1 bit data, 8 samples per byte. First sample in Most significant bit.
	ASIOSTDSDInt8NER8	= 40,		// DSD 8 bit data, 1 sample per byte. No Endianness required.

	ASIOSTLastEntry
};


using ASIOError = long;
enum {
	ASE_OK = 0,             // This value will be returned whenever the call succeeded
	ASE_SUCCESS = 0x3f4847a0,	// unique success return value for ASIOFuture calls
	ASE_NotPresent = -1000, // hardware input or output is not present or available
	ASE_HWMalfunction,      // hardware is malfunctioning (can be returned by any ASIO function)
	ASE_InvalidParameter,   // input parameter invalid
	ASE_InvalidMode,        // hardware is in a bad mode or used in a bad mode
	ASE_SPNotAdvancing,     // hardware is not running when sample position is inquired
	ASE_NoClock,            // sample clock or rate cannot be determined or is not present
	ASE_NoMemory            // not enough memory for completing the request
};

struct ASIODriverRegistration {
	std::wstring id;
	std::wstring name;
	std::wstring descr;
	std::wstring dllPath;

	inline std::wstring stringify() const {
		std::wstringstream ss;
		ss << L"<ASIODriverRegistration";
		ss << L" id=\"" << id << L"\"";
		ss << L" name=\"" << name << L"\"";
		ss << L" descr=\"" << descr << L"\"";
		ss << L" dllPath=\"" << dllPath << L"\"";
		ss << L">";
		return ss.str(); } };


struct ASIODriverInfo {
	long asioVersion; // currently, 2
	long driverVersion; // driver specific
	std::string name; };


#pragma pack(push,4)
struct ASIOTimeCode {
	double          speed;                  // speed relation (fraction of nominal speed)
	                                        // optional; set to 0. or 1. if not supported
	ASIOSamples     timeCodeSamples;        // time in samples
	unsigned long   flags;                  // some information flags (see below)
	char future[64]; };


enum ASIOTimeCodeFlags {
	kTcValid                = 1,
	kTcRunning              = 1 << 1,
	kTcReverse              = 1 << 2,
	kTcOnspeed              = 1 << 3,
	kTcStill                = 1 << 4,

	kTcSpeedValid           = 1 << 8 };


struct AsioTimeInfo {
	double          speed;                  // absolute speed (1. = nominal)
	ASIOTimeStamp   systemTime;             // system time related to samplePosition, in nanoseconds
	                                        // on mac, must be derived from Microseconds() (not UpTime()!)
	                                        // on windows, must be derived from timeGetTime()
	ASIOSamples     samplePosition;
	ASIOSampleRate  sampleRate;             // current rate
	unsigned long flags;                    // (see below)
	char reserved[12]; };


enum AsioTimeInfoFlags {
	kSystemTimeValid        = 1,            // must always be valid
	kSamplePositionValid    = 1 << 1,       // must always be valid
	kSampleRateValid        = 1 << 2,
	kSpeedValid             = 1 << 3,

	kSampleRateChanged      = 1 << 4,
	kClockSourceChanged     = 1 << 5 };


struct ASIOTime {                     // both input/output
	long reserved[4];                 // must be 0
	struct AsioTimeInfo timeInfo;     // required
	struct ASIOTimeCode timeCode; };  // optional, evaluated if (timeCode.flags & kTcValid)


struct ASIOCallbacks {
	void (*bufferSwitch) (void* ptr, long doubleBufferIndex, ASIOBool directProcess);
	void (*sampleRateDidChange) (void *ptr, ASIOSampleRate sRate);
	long (*asioMessage) (void *ptr, long selector, long value, void* message, double* opt);
	ASIOTime* (*bufferSwitchTimeInfo) (void *ptr, ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
	void* ptr;
	};


struct ASIOCallbacks_ABI {
	/**
	 * bufferSwitch indicates that both input and output are to be processed.
	 * the current buffer half index (0 for A, 1 for B) determines
	 * - the output buffer that the host should start to fill. the other buffer
	 * will be passed to output hardware regardless of whether it got filled
	 * in time or not.
	 * - the input buffer that is now filled with incoming data. Note that
	 * because of the synchronicity of i/o, the input always has at
	 * least one buffer latency in relation to the output.
	 * directProcess suggests to the host whether it should immedeately
	 * start processing (directProcess == ASIOTrue), or whether its process
	 * should be deferred because the call comes from a very low level
	 * (for instance, a high level priority interrupt), and direct processing
	 * would cause timing instabilities for the rest of the system. If in doubt,
	 * directProcess should be set to ASIOFalse.
	 * Note: bufferSwitch may be called at interrupt time for highest efficiency.
	 */
	void (*bufferSwitch) (long doubleBufferIndex, ASIOBool directProcess);

	/**
	 * called when the AudioStreamIO detects a sample rate change
	 * If sample rate is unknown, 0 is passed (for instance, clock loss
	 * when externally synchronized).
	 */
	void (*sampleRateDidChange) (ASIOSampleRate sRate);

	/**
	 * generic callback for various purposes, see selectors below.
	 * note this is only present if the asio version is 2 or higher
	 */
	long (*asioMessage) (long selector, long value, void* message, double* opt);

	/**
	 * new callback with time info. makes ASIOGetSamplePosition() and various
	 * calls to ASIOGetSampleRate obsolete,
	 * and allows for timecode sync etc. to be preferred; will be used if
	 * the driver calls asioMessage with selector kAsioSupportsTimeInfo.
	 */
	ASIOTime* (*bufferSwitchTimeInfo) (ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess); };


// asioMessage selectors
enum {
	kAsioSelectorSupported = 1,	// selector in <value>, returns 1L if supported,
								// 0 otherwise
    kAsioEngineVersion,			// returns engine (host) asio implementation version,
								// 2 or higher
	kAsioResetRequest,			// request driver reset. if accepted, this
								// will close the driver (ASIO_Exit() ) and
								// re-open it again (ASIO_Init() etc). some
								// drivers need to reconfigure for instance
								// when the sample rate changes, or some basic
								// changes have been made in ASIO_ControlPanel().
								// returns 1L; note the request is merely passed
								// to the application, there is no way to determine
								// if it gets accepted at this time (but it usually
								// will be).
	kAsioBufferSizeChange,		// not yet supported, will currently always return 0L.
								// for now, use kAsioResetRequest instead.
								// once implemented, the new buffer size is expected
								// in <value>, and on success returns 1L
	kAsioResyncRequest,			// the driver went out of sync, such that
								// the timestamp is no longer valid. this
								// is a request to re-start the engine and
								// slave devices (sequencer). returns 1 for ok,
								// 0 if not supported.
	kAsioLatenciesChanged, 		// the drivers latencies have changed. The engine
								// will refetch the latencies.
	kAsioSupportsTimeInfo,		// if host returns true here, it will expect the
								// callback bufferSwitchTimeInfo to be called instead
								// of bufferSwitch
	kAsioSupportsTimeCode,		//
	kAsioMMCCommand,			// unused - value: number of commands, message points to mmc commands
	kAsioSupportsInputMonitor,	// kAsioSupportsXXX return 1 if host supports this
	kAsioSupportsInputGain,     // unused and undefined
	kAsioSupportsInputMeter,    // unused and undefined
	kAsioSupportsOutputGain,    // unused and undefined
	kAsioSupportsOutputMeter,   // unused and undefined
	kAsioOverload,              // driver detected an overload

	kAsioNumMessageSelectors };


struct ASIOChannelInfo_ABI {
	long channel;			// on input, channel index
	ASIOBool isInput;		// on input
	ASIOBool isActive;		// on exit
	long channelGroup;		// dto
	ASIOSampleType type;	// dto
	char name[32]; };       // dto


struct ASIOChannelInfo {
	int channel;			// on input, channel index
	bool isInput;		// on input
	bool isActive;		// on exit
	int channelGroup;		// dto
	ASIOSampleType type;	// dto
	std::string name; };       // dto


struct ASIOBufferInfo {
	ASIOBool isInput;			// on input:  ASIOTrue: input, else output
	long channelNum;			// on input:  channel index
	void *buffers[2]; };		// on output: double buffer addresses

#pragma pack(pop)

class ASIOSystem {
private:
	ASIOSystem();
	~ASIOSystem();
	ASIOSystem(const ASIOSystem&) = delete;
	ASIOSystem& operator=(const ASIOSystem&) = delete;

public:
	static ASIOSystem& GetInstance();

	void RefreshDriverList();
	int FindDriverByName(const std::wstring& text) const;
	int FindDriverByName(const std::string& text) const;
	void OpenDriver(int idx);

	rqdq::ralio::ASIODriverInfo Init(void *sysRef);
	// on input: system reference
	// (Windows: application main window handle, Mac & SGI: 0)
	void ShowControlPanel();

	std::pair<int, int> GetChannels();
	std::tuple<int, int, int, int> GetBufferSize();
	double GetSampleRate();
	void SetSampleRate(double);
	bool SignalOutputReady();
	void CreateBuffers(ASIOBufferInfo*, int numChannels, int bufferSize, ASIOCallbacks* callbacks);
	ralio::ASIOChannelInfo GetChannelInfo(bool isInput, int channel);
	std::pair<int, int> GetLatencies();
	void Start();
	void Stop();
	void DisposeBuffers();
	void Exit() {}

public:
	std::vector<ASIODriverRegistration> d_drivers; };


class ASIOException : public std::runtime_error {
public:
	ASIOException(const char* message, int errorCode)
		:std::runtime_error{ message },
		d_errorCode{ errorCode } {}
	ASIOException(const std::string& message, int errorCode)
		:std::runtime_error{ message },
		d_errorCode{ errorCode } {}
	int ErrorCode() const noexcept {
		return d_errorCode; }
private:
	int d_errorCode; };


}  // close package namespace
}  // close enterprise namespace
