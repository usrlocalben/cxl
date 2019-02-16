#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_sampler.hxx"
#include "src/ral/ralio/ralio_asio.hxx"
#include "src/ral/ralm/ralm_grid_sequencer.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"
#include "src/rcl/rclw/rclw_console.hxx"

#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>

#define NOMINMAX
#include <Windows.h>


namespace rqdq {

namespace {

const double kTestRunTime = 6.0;
const int kNumVoices = 16;

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
		d_voices.reserve(kNumVoices);
		for (int i=0; i<kNumVoices; i++) {
			d_voices.emplace_back(d_waveTable);
			d_mixer.AddChannel(d_voices.back(), 1.0f);
			d_gridSequencer.AddTrack(d_voices.back()); }

		d_gridSequencer.InitializePattern();

		d_voices[0].d_params.waveId = 1;
		d_waveTable.Get(1) = ralw::MPCWave::Load("c:\\var\\lib\\cxl\\samples\\808 Kick_short.wav", "bd", false);

		d_voices[1].d_params.waveId = 2;
		d_waveTable.Get(2) = ralw::MPCWave::Load("c:\\var\\lib\\cxl\\samples\\808 Snare_lo1.wav", "sd", false);

		d_voices[2].d_params.waveId = 3;
		d_waveTable.Get(3) = ralw::MPCWave::Load("c:\\var\\lib\\cxl\\samples\\808 Hat_closed.wav", "ch", false);
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


const std::array<const std::string, 16> kTrackNames = {
	"BD", "SD", "HT", "MD", "LT", "CP", "RS", "CB",
	"CH", "OH", "RC", "CC", "M1", "M2", "M3", "M4" };


char tolower(char ch) {
	if ('A' <= ch && ch <= 'Z') {
		return ch - 'A' + 'a'; }
	return ch; }


const string& tolower(const string& s) {
	thread_local std::string tmp;
	tmp.clear();
	for (auto ch : s) {
		tmp.push_back(tolower(ch)); }
	return tmp; }


class FooMachineView {
public:
	FooMachineView(FooMachine& fooMachine, int selectedTrack, std::deque<std::string>& keyHistory) :d_fooMachine(fooMachine), d_selectedTrack(selectedTrack), d_keyHistory(keyHistory) {}

	void Draw(rclw::Console& console) {
		console
			.Position(1,0).Write("cxl 0.1.0")
			.Position(79-10,0).Write("anix/rqdq");
		DrawTrackSelection(console, 0, 2);
		DrawGrid(console, 1, 21);
		DrawKeyHistory(console, 60, 10);
		DrawTransportIndicator(console); }

	void DrawTrackSelection(rclw::Console& console, int x, int y) {
		console
			.Position(x, y)
			.LeftEdge(x);
		console.Write("  ");
		for (int i = 0; i < 8; i++) {
			console.Write(tolower(kTrackNames[i]) + " ");}
		console.CR();
		console.Write("  ");
		for (int i = 8; i <16; i++) {
			console.Write(tolower(kTrackNames[i]) + " ");}

		int selY = d_selectedTrack / 8       +  y;
		int selX = (d_selectedTrack % 8) * 3 + (x+2);
		console.Position(selX, selY).Write(kTrackNames[d_selectedTrack]);
		console.Position(selX-1, selY).Write("[");
		console.Position(selX+2, selY).Write("]"); }

	void DrawGrid(rclw::Console& console, int x, int y) {
		console.Position(x, y);
		console.Write("| .   .   .   . | .   .   .   . | .   .   .   . | .   .   .   . | ");
		console.Position(1, y+1);
		console.Write("| ");
		for (int i = 0; i < 16; i++) {
			auto value = d_fooMachine.d_gridSequencer.GetTrackGridNote(d_selectedTrack, i);
			console.Write(value ? "X" : " ");
			console.Write(" | "); }}

	void DrawKeyHistory(rclw::Console& console, int x, int y) {
		console.Position(x, y).LeftEdge(x);
		for (const auto& item : d_keyHistory) {
			console.Write(item).CR(); }}

	void DrawTransportIndicator(rclw::Console& console) {
		console.Position(79-8, 24).Write(d_fooMachine.d_gridSequencer.IsPlaying() ? "PLAYING" : "STOPPED"); }

	int d_selectedTrack = 0;
	std::deque<std::string>& d_keyHistory;
	FooMachine& d_fooMachine; };


constexpr uint32_t kCKRightAlt = 0x01;
constexpr uint32_t kCKLeftAlt = 0x02;
constexpr uint32_t kCKRightCtrl = 0x04;
constexpr uint32_t kCKLeftCtrl = 0x08;
constexpr uint32_t kCKShift = 0x10;

enum ScanCode {
	Esc = 1,
	Key1 = 2,
	Key2 = 3,
	Key3 = 4,
	Key4 = 5,
	Key5 = 6,
	Key6 = 7,
	Key7 = 8,
	Key8 = 9,
	Key9 = 10,
	Key0 = 11,
	Minus = 12,
	Equals = 13,
	Backspace = 14,
	Tab = 15,
	Q = 16,
	W = 17,
	E = 18,
	R = 19,
	T = 20,
	Y = 21,
	U = 22,
	I = 23,
	O = 24,
	P = 25,
	OpenBracket = 26,
	CloseBracket = 27,
	Enter = 28,
	LeftCtrl = 29,
	A = 30,
	S = 31,
	D = 32,
	F = 33,
	G = 34,
	H = 35,
	J = 36,
	K = 37,
	L = 38,
	Semicolon = 39,
	Quote = 40,
	Tilde = 41,
	LeftShift = 42,
	Backslash = 43,
	Z = 44,
	X = 45,
	C = 46,
	V = 47,
	B = 48,
	N = 49,
	M = 50,
	Comma = 51,
	Period = 52,
	ForwardSlash = 53,
	RightShift = 54,
	// 55 ?
	Alt = 56,  // same scancode for left&right? but flags are different... strange
	Space = 57,
    /// ...
	LeftWindows = 91, };



int main(int argc, char **argv) {
	cout << "==== BEGIN ====\n" << flush;



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

	auto& console = rclw::Console::GetInstance();

	console.SetDimensions(80, 25);
	console.Clear();



	std::vector<HANDLE> pendingEvents;


	bool done = false;

	int selectedTrack = 0;
	std::deque<std::string> keyHistory;
	std::vector<bool> downKeys(256, false);

	while (!done) {
		FooMachineView(fooMachine, selectedTrack, keyHistory).Draw(console);

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

					{stringstream ss;
					ss << (e.bKeyDown?'D':'U');
					ss << " " << hex << e.dwControlKeyState << dec;
					ss << " " << e.uChar.AsciiChar;
					ss << " " << e.wRepeatCount;
					ss << " " << e.wVirtualKeyCode;
					ss << " " << e.wVirtualScanCode << "  ";
					keyHistory.emplace_back(ss.str());
					if (keyHistory.size() > 8) {
						keyHistory.pop_front(); }}

					if (e.wVirtualScanCode<256) {
						downKeys[e.wVirtualScanCode] = e.bKeyDown; }

					if (e.bKeyDown && e.dwControlKeyState==kCKLeftCtrl && e.wVirtualScanCode==ScanCode::Q) {
						done = true; }
					else if (e.bKeyDown && e.dwControlKeyState==kCKLeftCtrl && (ScanCode::Key1<=e.wVirtualScanCode && e.wVirtualScanCode<=ScanCode::Key8)) {
						// Ctrl+1...Ctrl+8
						selectedTrack = e.wVirtualScanCode - ScanCode::Key1; }
					else if (e.bKeyDown && e.dwControlKeyState==0) {
						if (e.wVirtualScanCode == ScanCode::Semicolon) { fooMachine.Stop(); }
						else if (e.wVirtualScanCode == ScanCode::Quote) { fooMachine.Play(); }
						else if (e.wVirtualScanCode == ScanCode::Comma || e.wVirtualScanCode == ScanCode::Period) {

							int adj = (e.wVirtualScanCode == ScanCode::Comma ? -1 : 1);
							if (e.dwControlKeyState & kCKShift) adj *= 10;

							if (downKeys[ScanCode::T]) {
								fooMachine.d_voices[selectedTrack].d_params.cutoff += adj; }
							else if (downKeys[ScanCode::Y]) {
								fooMachine.d_voices[selectedTrack].d_params.resonance += adj; }
							else if (downKeys[ScanCode::U]) {
								fooMachine.d_voices[selectedTrack].d_params.attackPct += adj; }
							else if (downKeys[ScanCode::I]) {
								fooMachine.d_voices[selectedTrack].d_params.decayPct += adj; }
							else if (downKeys[ScanCode::Equals]) {
								fooMachine.d_gridSequencer.SetTempo(fooMachine.d_gridSequencer.GetTempo() + adj); }}
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

