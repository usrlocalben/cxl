#include "src/ral/ralio/ralio_asio.hxx"

#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rclw/rclw_guid.hxx"
#include "src/rcl/rclw/rclw_smarti.hxx"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "3rdparty/asiosdk/common/iasiodrv.h"
#include "3rdparty/winreg/WinReg.hxx"


namespace rqdq {

namespace {

IASIO* theAsioDriver = nullptr;
rqdq::ralio::ASIOCallbacks* theCallbacks = nullptr;

}  // close unnamed namespace

namespace ralio {

ASIOSystem::ASIOSystem() {
	CoInitialize(nullptr); }


ASIOSystem& ASIOSystem::GetInstance() {
	static ASIOSystem instance;
	return instance; }


ASIOSystem::~ASIOSystem() {
	if (theAsioDriver != nullptr) {
		Stop(/*throwOnError=*/false);
		theAsioDriver->Release();
		theAsioDriver = nullptr; }
	CoUninitialize(); }


void ASIOSystem::RefreshDriverList() {
	using RegKey = winreg::RegKey;
	d_drivers.clear();

	//std::cerr << "getting ASIO driver list\n";
	RegKey driverListNode{ HKEY_LOCAL_MACHINE, L"software\\ASIO", KEY_READ };
	for (const auto& driverName : driverListNode.EnumSubKeys()) {

		//std::wcerr << L"compiling \"" << driverName << L"\"\n";
		RegKey driverNode{ driverListNode.Get(), driverName, KEY_READ };
		try {
			auto clsIdText = driverNode.GetStringValue(L"CLSID");
			rclw::CLSIDSerializer::Deserialize(clsIdText);
			auto driverId = clsIdText.substr(1, clsIdText.length()-2);

			RegKey comNode{ HKEY_CLASSES_ROOT, L"CLSID\\" + clsIdText + L"\\InprocServer32", KEY_READ };
			auto dllPath = comNode.GetStringValue(L"");

			std::wstring descr;
			try {
				descr = driverNode.GetStringValue(L"description"); }
			catch (...) {
				descr = driverName; }

			rcls::EnsureOpenable(dllPath);
			const rqdq::ralio::ASIODriverRegistration entry{ driverId, driverName, descr, dllPath };
			d_drivers.emplace_back(entry); }
		catch (const std::exception& err) {
			std::wcerr << L"ignoring \"" << driverName << L"\": ";
			std::cerr << err.what() << "\n"; }}}


int ASIOSystem::FindDriverByName(const std::wstring& text) const {
	auto search = std::find_if(begin(d_drivers), end(d_drivers), [&](auto& item) { return item.name == text; });
	if (search == end(d_drivers)) {
		return -1; }
	return std::distance(begin(d_drivers), search); }


int ASIOSystem::FindDriverByName(const std::string& text) const {
	return FindDriverByName(rclt::UTF8Codec::Decode(text)); }


void ASIOSystem::OpenDriver(int idx) {
	void* vtbl;
	auto& info = d_drivers[idx];
	const auto clsId = rclw::CLSIDSerializer::Deserialize(L"{" + info.id + L"}");
	int result = CoCreateInstance(clsId, nullptr, CLSCTX_INPROC_SERVER, clsId, &vtbl);
	if (result != S_OK) {
		throw std::runtime_error("could not create driver instance"); }
	theAsioDriver = static_cast<IASIO*>(vtbl);
	std::cerr << "driver open\n"; }


void EnsureDriverOpen() {
	if (theAsioDriver == nullptr) {
		throw std::runtime_error("driver not open"); }}


rqdq::ralio::ASIODriverInfo ASIOSystem::Init(void* sysRef) {
	rqdq::ralio::ASIODriverInfo out;
	out.asioVersion = 2;
	out.driverVersion = 0;
	out.name = "Unknown";

	EnsureDriverOpen();

	int result = theAsioDriver->init(sysRef);
	if (result == 0) {
		char buf[124];
		theAsioDriver->getErrorMessage(buf);
		theAsioDriver = nullptr;
		throw std::runtime_error(buf); }

	{
		char buf[32];
		theAsioDriver->getDriverName(buf);
		out.name.assign(buf); }

	out.driverVersion = theAsioDriver->getDriverVersion();
	return out; }


std::pair<int, int> ASIOSystem::GetChannels() {
	EnsureDriverOpen();
	long chIn = 0;
	long chOut = 0;
	int result = theAsioDriver->getChannels(&chIn, &chOut);
	if (result != ASE_OK) {
		throw ASIOException{"getChannels failed", result}; }
	return {chIn, chOut}; }


std::tuple<int, int, int, int> ASIOSystem::GetBufferSize() {
	EnsureDriverOpen();
	long a, b, c, d;
	int result = theAsioDriver->getBufferSize(&a, &b, &c, &d);
	if (result != ASE_OK) {
		throw ASIOException("getBufferSize failed", result); }
	return {a, b, c, d}; }


double ASIOSystem::GetSampleRate() {
	EnsureDriverOpen();
	double rate;
	int result = theAsioDriver->getSampleRate(&rate);
	if (result != ASE_OK) {
		throw ASIOException("getSampleRate failed", result); }
	return rate; }


void ASIOSystem::SetSampleRate(double rate) {
	EnsureDriverOpen();
	int result = theAsioDriver->setSampleRate(rate);
	if (result != ASE_OK) {
		throw ASIOException("setSampleRate failed", result); }}


bool ASIOSystem::SignalOutputReady() {
	EnsureDriverOpen();
	int result = theAsioDriver->outputReady();
	if (result == ASE_NotPresent) {
		return false; }
	if (result == ASE_OK) {
		return true; }
	throw ASIOException("outputReady failed", result); }


void bufferSwitchJmp(long arg1, ASIOBool arg2) {
	theCallbacks->bufferSwitch(theCallbacks->ptr, arg1, arg2); }

void sampleRateDidChangeJmp(ASIOSampleRate arg1) {
	theCallbacks->sampleRateDidChange(theCallbacks->ptr, arg1); }

long asioMessageJmp(long arg1, long arg2, void* arg3, double* arg4) {
	return theCallbacks->asioMessage(theCallbacks->ptr, arg1, arg2, arg3, arg4); }

ASIOTime* bufferSwitchTimeInfoJmp(ASIOTime* arg1, long arg2, ASIOBool arg3) {
	return theCallbacks->bufferSwitchTimeInfo(theCallbacks->ptr, arg1, arg2, arg3); }


void ASIOSystem::CreateBuffers(rqdq::ralio::ASIOBufferInfo *bufs, int numChannels, int bufferSize, rqdq::ralio::ASIOCallbacks* callbacks) {
	EnsureDriverOpen();
	theCallbacks = callbacks;
	static ASIOCallbacks_ABI jmpCallbacks;
	jmpCallbacks.bufferSwitch = &bufferSwitchJmp;
	jmpCallbacks.sampleRateDidChange = &sampleRateDidChangeJmp;
	jmpCallbacks.asioMessage = &asioMessageJmp;
	jmpCallbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfoJmp;

	int result = theAsioDriver->createBuffers(reinterpret_cast<::ASIOBufferInfo*>(bufs), numChannels, bufferSize, reinterpret_cast<::ASIOCallbacks*>(&jmpCallbacks));
	if (result != ASE_OK) {
		throw ASIOException("createBuffers failed", result); }}


rqdq::ralio::ASIOChannelInfo ASIOSystem::GetChannelInfo(bool isInput, int channel) {
	EnsureDriverOpen();
	rqdq::ralio::ASIOChannelInfo_ABI info{};
	info.isInput = static_cast<rqdq::ralio::ASIOBool>(isInput);
	info.channel = channel;
	int result = theAsioDriver->getChannelInfo(reinterpret_cast<::ASIOChannelInfo*>(&info));
	if (result != ASE_OK) {
		throw ASIOException("getChannelInfo failed", result); }

	return rqdq::ralio::ASIOChannelInfo{
		info.channel,
		bool(info.isInput),
		bool(info.isActive),
		info.channelGroup,
		info.type,
		info.name
		};}


std::pair<int, int> ASIOSystem::GetLatencies() {
	EnsureDriverOpen();
	long a, b;
	int result = theAsioDriver->getLatencies(&a, &b);
	if (result != ASE_OK) {
		throw ASIOException("getLatencies failed", result); }
	return {a, b}; }


void ASIOSystem::Start() {
	EnsureDriverOpen();
	int result = theAsioDriver->start();
	if (result != ASE_OK) {
		throw ASIOException("start failed", result); }}


void ASIOSystem::Stop(bool throwOnError/*=true*/) {
	EnsureDriverOpen();
	int result = theAsioDriver->stop();
	if (result != ASE_OK && throwOnError) {
		throw ASIOException("stop failed", result); }}


void ASIOSystem::DisposeBuffers() {
	EnsureDriverOpen();
	int result = theAsioDriver->disposeBuffers();
	if (result != ASE_OK) {
		throw ASIOException("disposeBuffers failed", result); }}


void ASIOSystem::ShowControlPanel() {
	EnsureDriverOpen();
	int result = theAsioDriver->controlPanel();
	if (result != ASE_OK) {
		throw ASIOException("controlPanel failed", result); }}


}  // namespace ralio
}  // namespace rqdq
