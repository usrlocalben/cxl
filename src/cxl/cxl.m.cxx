#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_sampler.hxx"
#include "src/ral/ralio/ralio_asio.hxx"
#include "src/ral/ralm/ralm_grid_sequencer.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"
#include "src/rcl/rclw/rclw_console.hxx"

#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>

#define NOMINMAX
#include <Windows.h>


namespace rqdq {

namespace {

const double kTestRunTime = 6.0;
const int kNumVoices = 6;

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

int inputLatency;
int outputLatency;

// output ready
bool supportsOutputReadyOptimization;

// buffers and channels
std::vector<ralio::ASIOBufferInfo> bufferInfos;
std::vector<ralio::ASIOChannelInfo> channelInfos;

std::mutex foolock;
// get sample position
double nanoSeconds;
double samples;
double tcSamples;

// bufferSwitchTimeInfo
ralio::ASIOTime tInfo;
uint32_t sysRefTime;
uint32_t tstart;

bool stopped;

}  // close unnamed namespace

namespace cxl {

using namespace std;

double fract(double x) {
	return x - (int64_t)x; }


class FooMachine {
public:
	FooMachine() :d_waveTable(16) {
		d_gridSequencer.SetTempo(800);
		d_voices.reserve(6);
		for (int i=0; i<kNumVoices; i++) {
			d_voices.emplace_back(d_waveTable);
			d_mixer.AddChannel(d_voices.back(), 1.0f);
			d_gridSequencer.AddTrack(d_voices.back()); }

		d_voices[1].d_params.waveId = 1;
		d_voices[2].d_params.waveId = 1;

		// clear all grids
		for (int ti=0; ti<kNumVoices; ti++) {
			for (int i=0; i<16; i++) {
				d_gridSequencer.SetTrackGridNote(ti, i, 0); }}

		d_voices[0].d_params.waveId = 1;
		d_waveTable.Get(1) = ralw::MPCWave::Load("c:\\var\\lib\\cxl\\samples\\808 Kick_short.wav", "bd", false);
		d_gridSequencer.SetTrackGridNote(0, 0, 1);
		d_gridSequencer.SetTrackGridNote(0, 6, 1);
		//d_gridSequencer.SetTrackGridNote(0, 8, 1);
		//d_gridSequencer.SetTrackGridNote(0, 12, 1);

		d_voices[1].d_params.waveId = 2;
		d_waveTable.Get(2) = ralw::MPCWave::Load("c:\\var\\lib\\cxl\\samples\\808 Snare_lo1.wav", "sd", false);
		d_gridSequencer.SetTrackGridNote(1, 4, 1);
		d_gridSequencer.SetTrackGridNote(1, 12, 1);

		d_voices[2].d_params.waveId = 3;
		d_waveTable.Get(3) = ralw::MPCWave::Load("c:\\var\\lib\\cxl\\samples\\808 Hat_closed.wav", "ch", false);
		d_gridSequencer.SetTrackGridNote(2, 0, 1);
		d_gridSequencer.SetTrackGridNote(2, 2, 1);
		d_gridSequencer.SetTrackGridNote(2, 4, 1);
		d_gridSequencer.SetTrackGridNote(2, 6, 1);
		d_gridSequencer.SetTrackGridNote(2, 8, 1);
		d_gridSequencer.SetTrackGridNote(2, 10, 1);
		d_gridSequencer.SetTrackGridNote(2, 12, 1);
		//d_gridSequencer.SetTrackGridNote(2, 13, 1);
		d_gridSequencer.SetTrackGridNote(2, 14, 1);
		d_gridSequencer.SetTrackGridNote(2, 15, 1);
	}

	rqdq::ralio::ASIOCallbacks MakeASIOCallbacks() {
		rqdq::ralio::ASIOCallbacks out;
		out.bufferSwitch = &FooMachine::onBufferReadyJmp;
		out.bufferSwitchTimeInfo = &FooMachine::onBufferReadyExJmp;
		out.sampleRateDidChange = &FooMachine::onSampleRateChangedJmp;
		out.asioMessage = &FooMachine::onASIOMessageJmp;
		out.ptr = reinterpret_cast<void*>(this);
		return out; }

	ralio::ASIOTime* onBufferReadyEx(ralio::ASIOTime* timeInfo, long index, ralio::ASIOBool processNow) {
		using namespace rqdq::ralio;
		static long processedSamples = 0;
		std::scoped_lock lock(foolock);

		d_gridSequencer.Update();

		// store the timeInfo for later use
		tInfo = *timeInfo;

		// get the time stamp of the buffer, not necessary if no
		// synchronization to other media is required
		nanoSeconds = 0;
		samples = 0;
		tcSamples = 0;
		if (timeInfo->timeInfo.flags & kSystemTimeValid)
			nanoSeconds = timeInfo->timeInfo.systemTime;
		if (timeInfo->timeInfo.flags & kSamplePositionValid)
			samples = timeInfo->timeInfo.samplePosition;
		if (timeInfo->timeCode.flags & kTcValid)
			tcSamples = timeInfo->timeCode.timeCodeSamples;

		// get the system reference time
		sysRefTime = timeGetTime();

		long buffSize = bufPreferredSize;  // buffer size in samples

		dl.resize(buffSize);
		dr.resize(buffSize);
		for (int si = 0; si < buffSize; si++) {
			d_gridSequencer.Tick();
			std::tie(dl[si], dr[si]) = d_mixer.GetNextSample(); }


		// perform the processing
		for (int i=0; i<numInputChannels+numOutputChannels; i++) {

			if (bufferInfos[i].isInput == false) {
				if (i==1) continue;
				//cout << "CT(" << int(channelInfos[i].type) << ")";
				// OK do processing for the outputs only
				switch (channelInfos[i].type) {
				case ASIOSTInt16LSB:
					memset (bufferInfos[i].buffers[index], 0, buffSize * 2);
					break;
				case ASIOSTInt24LSB:		// used for 20 bits as well
					memset (bufferInfos[i].buffers[index], 0, buffSize * 3);
					break;
				case ASIOSTInt32LSB:
					//memset (bufferInfos[i].buffers[index], 0, buffSize * 4);
					{auto dst = reinterpret_cast<int*>(bufferInfos[i].buffers[index]);
					for (int si=0; si<buffSize; si++) {
						double t = (processedSamples + si) / 44100.0;
						double phase = fract(t);
						double hz = 440.0 + sin(phase*2*3.14159262) * 50.0;
						dst[si] = sin(fract(t)*hz*2*3.14159262) * 0x7fffffff; }}
					break;
				case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
					//memset (bufferInfos[i].buffers[index], 0, buffSize * 4);
					{auto dst = reinterpret_cast<float*>(bufferInfos[i].buffers[index]);
					for (int si=0; si<buffSize; si++) { dst[si] = dl[si];}}
					break;
				case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
					memset (bufferInfos[i].buffers[index], 0, buffSize * 8);
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

		//if (processedSamples >= sampleRate * kTestRunTime)	// roughly measured
		//	stopped = true;
		//else
			processedSamples += buffSize;

		return 0L; }

	void onBufferReady(long index, ralio::ASIOBool processNow) {
		using namespace ralio;
		ASIOTime timeInfo;
		memset(&timeInfo, 0, sizeof (timeInfo));

		// get the time stamp of the buffer, not necessary if no
		// synchronization to other media is required
		//if (asio.OGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		//	timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

		onBufferReadyEx(&timeInfo, index, processNow); }

	void onSampleRateChanged(ralio::ASIOSampleRate sRate) {}

	/**
	 * asio message handler callback
	 */
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
				stopped;  // In this sample the processing will just stop
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

	static ralio::ASIOTime* onBufferReadyExJmp(void* ptr, ralio::ASIOTime *timeInfo, long index, ralio::ASIOBool processNow) {
		auto& self = *reinterpret_cast<FooMachine*>(ptr);
		return self.onBufferReadyEx(timeInfo, index, processNow); }
	static void onBufferReadyJmp(void* ptr, long index, ralio::ASIOBool processNow) {
		auto& self = *reinterpret_cast<FooMachine*>(ptr);
		return self.onBufferReady(index, processNow); }
	static void onSampleRateChangedJmp(void* ptr, ralio::ASIOSampleRate sRate) {
		auto& self = *reinterpret_cast<FooMachine*>(ptr);
		return self.onSampleRateChanged(sRate); }
	static long onASIOMessageJmp(void* ptr, long selector, long value, void* message, double* opt) {
		auto& self = *reinterpret_cast<FooMachine*>(ptr);
		return self.onASIOMessage(selector, value, message, opt); }

	void Play() {
		d_gridSequencer.Play(); }
	void Stop() {
		d_gridSequencer.Stop(); }

public:
	std::vector<float> dl, dr;
	ralw::WaveTable d_waveTable;
	raldsp::BasicMixer d_mixer;
	ralm::GridSequencer d_gridSequencer;
	std::vector<raldsp::SingleSampler> d_voices; };


constexpr uint32_t kCKRightAlt = 0x01;
constexpr uint32_t kCKLeftAlt = 0x02;
constexpr uint32_t kCKRightCtrl = 0x04;
constexpr uint32_t kCKLeftCtrl = 0x08;
constexpr uint32_t kCKShift = 0x10;


int main(int argc, char **argv) {
	cout << "==== BEGIN ====\n" << flush;

	/*
	auto& console = rclw::Console::GetInstance();

	console.SetDimensions(80, 25);
	console.Clear();

	console
		.Position(2, 2)
		.LeftEdge(2)
		.Write("Tempo: 120.0").CR()
		.Write("Bar: 1/1").CR()
		.CR()
		.PushPosition()
		.Write("BD [X| | | | | |X| | | | | | | | | ]").CR()
		.Write("SD [ | | | |X| | | | | | | |X| | | ]").CR()
		.Write("CH [X| |X| |X| |X| |X| |X| |X| |X| ]").CR()
		.Write("MT [ | | |X| | | | | | | |X| | | | ]").CR()
		.Position(40,2)
		.LeftEdge(40)
		.Write("Voice: SD").CR()
		.Write("--------------").CR()
		.Write("Attack: 002").CR()
		.Write("Decay:  030").CR()
		.Write("Pitch:  +06").CR()
		.Write("Cutoff: 100").CR()
		.Write("Res:    000").CR()
		.PopPosition()
		.ShowCursor();

	//Sleep(10000);
	//return 0;
	*/

	tstart = timeGetTime();

	auto& asio = ralio::ASIOSystem::GetInstance();
	asio.RefreshDriverList();

	std::for_each(begin(asio.d_drivers), end(asio.d_drivers), [](auto& item) {
				  wcout << item.stringify() << "\n"; });

	//int idx = asio.FindDriverByName("ASIO4ALL v2");
	int idx = asio.FindDriverByName("FlexASIO");
	if (idx == -1) {
		throw std::runtime_error("driver not found"); }

	asio.OpenDriver(idx);
	const auto info = asio.Init(0);

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

	FooMachine fooMachine;
	auto asioCallbacks = fooMachine.MakeASIOCallbacks();


	bufferInfos.resize(numInputChannels+numOutputChannels);
	channelInfos.resize(numInputChannels+numOutputChannels);
	idx = 0;
	for (int i=0; i<numInputChannels; i++) {
		auto& info = bufferInfos[idx++];
		info.isInput = ralio::ASIOTrue;
		info.channelNum = i;
		info.buffers[0] = nullptr;
		info.buffers[1] = nullptr; }
	for (int i=0; i<numOutputChannels; i++) {
		auto& info = bufferInfos[idx++];
		info.isInput = ralio::ASIOFalse;
		info.channelNum = i;
		info.buffers[0] = nullptr;
		info.buffers[1] = nullptr; }

	asio.CreateBuffers(
		bufferInfos.data(),
		numInputChannels + numOutputChannels,
		bufPreferredSize,
		&asioCallbacks
		);

	for (int i=0; i<numInputChannels+numOutputChannels; i++) {
		channelInfos[i] = asio.GetChannelInfo(bufferInfos[i].isInput, bufferInfos[i].channelNum);
		{
			const auto& info = channelInfos[i];
			cout << "Ch" << info.channel << ", " << (info.isInput?" INPUT":"OUTPUT") << " \"" << info.name << "\", " << info.type << endl;
		}

	}

	std::tie(inputLatency, outputLatency) = asio.GetLatencies();


	asio.Start();

	std::vector<HANDLE> pendingEvents;

	fooMachine.Play();

	bool done = false;

	int selectedTrack = 0;

	while (!done) {
		pendingEvents.clear();
		pendingEvents.emplace_back(GetStdHandle(STD_INPUT_HANDLE));
		DWORD result = WaitForMultipleObjects(pendingEvents.size(), pendingEvents.data(), FALSE, 1000);
		if (result == WAIT_TIMEOUT) {
			// nothing
			}
		else if (WAIT_OBJECT_0 <= result && result < (WAIT_OBJECT_0+pendingEvents.size())) {
			int firstSignaledIdx = result - WAIT_OBJECT_0;
			if (firstSignaledIdx == 0) {
				// process stdin
				INPUT_RECORD record;
				DWORD numRead;
				if (!ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &record, 1, &numRead)) {
					throw std::runtime_error("ReadConsoleInput failure"); }
				if (record.EventType == KEY_EVENT) {
					const auto& e = record.Event.KeyEvent;

					{cout << (e.bKeyDown?'D':'U');
					cout << " " << hex << e.dwControlKeyState << dec;
					cout << " " << e.uChar.AsciiChar;
					cout << " " << e.wRepeatCount;
					cout << " " << e.wVirtualKeyCode;
					cout << " " << e.wVirtualScanCode;
					cout << endl;}

					if (e.bKeyDown && e.dwControlKeyState==kCKLeftCtrl && e.wVirtualScanCode==16) {
						// Ctrl+Q
						done = true; }
					else if (e.bKeyDown && e.dwControlKeyState==kCKLeftCtrl && (2<=e.wVirtualScanCode && e.wVirtualScanCode<=7)) {
						// Ctrl+1...Ctrl+6
						selectedTrack = e.wVirtualScanCode - 2; }
					else if (e.bKeyDown && e.dwControlKeyState==0) {
						if (e.wVirtualScanCode == 39) {
							// semicolon
							fooMachine.Stop(); }
						else if (e.wVirtualScanCode == 40) {
							// single-quote
							fooMachine.Play(); }
						else {
							const array<int, 16> gridscan = { 2, 3, 4, 5, 16, 17, 18, 19, 30, 31, 32, 33, 44, 45, 46, 47 };
							int i;
							for (i=0; i<16; i++) {
								if (e.wVirtualScanCode == gridscan[i]) {
									break; }}
							if (i<16) {
								fooMachine.d_gridSequencer.ToggleTrackGridNote(selectedTrack, i); }}}

				}}}}

	/*

	cout << "ASIO Driver started successfully.\n";
	while (!stopped) {
		Sleep(100);
		/*std::scoped_lock lock(foolock);
		cout << sysRefTime-tstart << " ms";
		cout << " / " << fixed << (nanoSeconds/1000000.0) << " ms";
		cout << " / " << (long)(nanoSeconds / 1000000.0) << " ms";
		cout << " / " << samples << " samples";

		cout << "      \r" << flush;
		}
		*/

	asio.Stop();
	asio.DisposeBuffers();
	asio.Exit();
	cout << "==== END ====\n" << flush;
	return 0; }


}  // close package namespace
}  // close enterprise namespace


int main(int argc, char **argv) {
	int result;
	try {
		result = rqdq::cxl::main(argc, argv); }
	catch (const std::exception& err) {
		std::cout << "exception: " << err.what() << std::endl;
		result = EXIT_FAILURE; }
	exit(result); }
