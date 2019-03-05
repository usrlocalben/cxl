#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/cxl_config.hxx"
#include "src/cxl/cxl_reactor.hxx"
#include "src/cxl/cxl_log.hxx"
#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_sampler.hxx"
#include "src/ral/ralm/ralm_grid_sequencer.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"
#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

#include <utility>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "3rdparty/fmt/include/fmt/format.h"
#include "3rdparty/fmt/include/fmt/printf.h"


namespace rqdq {
namespace {

constexpr int kNumVoices = 16;
constexpr int kMaxWaves = 1024;
constexpr int kDefaultTempo = 1200;
constexpr float kMasterGain = 0.666f;
constexpr int kMaxKitNum = 99;


const std::string& MakeKitPath(int n) {
	static std::string out;
	out = rcls::JoinPath(cxl::config::kitDir, fmt::sprintf("%02d.kit", n));
	return out; }


const std::string& MakePatternPath(int n) {
	static std::string out;
	out = rcls::JoinPath(cxl::config::patternDir, fmt::sprintf("A%02d.pattern", n));
	return out; }


}  // namespace
namespace cxl {

void CXLEffects::Update(int tempo) {
	const int freq = d_lowpassFreq;
	const int q = d_lowpassQ;
	if (freq == 127) {
		d_filter.SetBypass(true); }
	else {
		d_filter.SetBypass(false);
		d_filter.SetCutoff(pow(freq/127.0f, 2.0f));
		// scale Q-max slightly under 1.0
		d_filter.SetQ(sqrt(q/128.9f)); }

	d_filter.Update(tempo);

	d_delay.SetTime(d_delayTime);
	d_delay.SetFeedbackGain(d_delayFeedback / 127.0);
	d_delay.Update(tempo);

	d_reducer.d_midi = d_reduce;
	d_reducer.Update(tempo); }


void CXLEffects::Process(float* inputs, float* outputs) {
	float filtered;
	d_filter.Process(inputs, &filtered);

	float delaySend = d_delaySend / 127.0;
	float toDelay = filtered * delaySend;
	float fromDelay;
	d_delay.Process(&toDelay, &fromDelay);

	float filteredPlusDelay = filtered + fromDelay;
	d_reducer.Process(&filteredPlusDelay, outputs); }


CXLUnit::CXLUnit()
	:d_waveTable(kMaxWaves) {
	d_sequencer.SetTempo(kDefaultTempo);
	d_voices.reserve(kNumVoices);
	d_effects.reserve(kNumVoices);
	for (int i=0; i<kNumVoices; i++) {
		d_voices.emplace_back(d_waveTable);
		d_effects.emplace_back();
		d_mixer.AddChannel();
		d_sequencer.AddTrack(d_voices.back()); }

	BeginLoadingWaves(); }


void CXLUnit::BeginLoadingWaves() {
	d_loading = true;
	d_filesToLoad = rcls::FindGlob(rcls::JoinPath(config::sampleDir, R"(*.wav)"));
	sort(begin(d_filesToLoad), end(d_filesToLoad));
	d_nextFileId = 0;
	d_nextWaveId = 1;
	d_loaderStateChanged.emit();
	MakeProgressLoadingWaves(); }


void CXLUnit::MakeProgressLoadingWaves() {
	if (d_nextFileId >= d_filesToLoad.size()) {
		return; }
	auto& reactor = Reactor::GetInstance();
	const auto wavPath = rcls::JoinPath(config::sampleDir, d_filesToLoad[d_nextFileId]);
	auto& lfd = reactor.LoadFile(wavPath);
	lfd.AddCallbacks([&](std::vector<uint8_t>& data) { this->onWaveIOComplete(data); },
	                 [&](uint32_t err) { this->onWaveIOError(err); }); }


float CXLUnit::GetLoadingProgress() {
	float total = d_filesToLoad.size();
	float done = d_nextFileId;
	return done / total; }


void CXLUnit::onWaveIOComplete(const std::vector<uint8_t>& data) {
	const int fileId = d_nextFileId++;
	const int waveId = d_nextWaveId++;
	MakeProgressLoadingWaves();

	auto& fileName = d_filesToLoad[fileId];
	auto baseName = fileName.substr(0, fileName.size() - 4);
	d_waveTable.Get(waveId) = ralw::MPCWave::Load(data, baseName);
	Log::GetInstance().info(fmt::sprintf("loaded wave %d \"%s\"", d_nextWaveId, baseName));
	d_loaderStateChanged.emit();

	if (d_nextFileId >= d_filesToLoad.size()) {
		onLoadingComplete(); }}


void CXLUnit::onLoadingComplete() {
	d_loading = false;
	d_loaderStateChanged.emit();
	SwitchPattern(0); }


void CXLUnit::onWaveIOError(int error) {
	d_nextFileId++;
	MakeProgressLoadingWaves(); }


// transport controls
void CXLUnit::Play() {
	d_sequencer.Play(); }


void CXLUnit::Stop() {
	d_sequencer.Stop(); }


bool CXLUnit::IsPlaying() {
	return d_sequencer.IsPlaying(); }


int CXLUnit::GetLastPlayedGridPosition() {
	return d_sequencer.GetLastPlayedGridPosition(); }


void CXLUnit::SetTempo(int value) {
	d_sequencer.SetTempo(value);
	d_tempoChanged.emit(value); }


int CXLUnit::GetTempo() {
	return d_sequencer.GetTempo(); }


// pattern editing
void CXLUnit::ToggleTrackGridNote(int track, int pos) {
	d_sequencer.ToggleTrackGridNote(track, pos);
	d_patternDataChanged.emit(track); }


int CXLUnit::GetTrackGridNote(int track, int pos) {
	return d_sequencer.GetTrackGridNote(track, pos); }


void CXLUnit::SetTrackGridNote(int track, int pos, int note) {
	return d_sequencer.SetTrackGridNote(track, pos, note); }


const std::string CXLUnit::GetVoiceParameterName(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	case 0: return "wav";
	case 1: return "atk";
	case 2: return "dcy";

	case 4: return "tun";
	case 5: return "fin";
	default: return ""; }}


int CXLUnit::GetVoiceParameterValue(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	case 0: return d_voices[ti].d_waveId;
	case 1: return d_voices[ti].d_attackPct;
	case 2: return d_voices[ti].d_decayPct;

	case 4: return d_voices[ti].d_tuningInNotes;
	case 5: return d_voices[ti].d_tuningInCents;
	default: return 0; }}


void CXLUnit::AdjustVoiceParameter(int ti, int pi, int offset) {
	switch (pi) {
	case 0: Adjust2(ti, d_voices[ti].d_waveId,          0, 1000, offset); break;
	case 1: Adjust2(ti, d_voices[ti].d_attackPct,       0,  100, offset); break;
	case 2: Adjust2(ti, d_voices[ti].d_decayPct,        0,  100, offset); break;

	case 4: Adjust2(ti, d_voices[ti].d_tuningInNotes, -64,   63, offset); break;
	case 5: Adjust2(ti, d_voices[ti].d_tuningInCents,-100,  100, offset); break;
	default: break; }}


const std::string CXLUnit::GetEffectParameterName(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	case 0: return "flt";
	case 1: return "rez";
	case 2: return "dly";
	case 3: return "dtm";
	case 4: return "dfb";

	case 7: return "red";
	default: return ""; }}


int CXLUnit::GetEffectParameterValue(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	case 0: return d_effects[ti].d_lowpassFreq;
	case 1: return d_effects[ti].d_lowpassQ;
	case 2: return d_effects[ti].d_delaySend;
	case 3: return d_effects[ti].d_delayTime;
	case 4: return d_effects[ti].d_delayFeedback;

	case 7: return d_effects[ti].d_reduce;
	default: return 0; }}


void CXLUnit::AdjustEffectParameter(int ti, int pi, int offset) {
	switch (pi) {
	case 0: Adjust2(ti, d_effects[ti].d_lowpassFreq,   0,  127, offset); break;
	case 1: Adjust2(ti, d_effects[ti].d_lowpassQ,      0,  127, offset); break;
	case 2: Adjust2(ti, d_effects[ti].d_delaySend,     0,  127, offset); break;
	case 3: Adjust2(ti, d_effects[ti].d_delayTime,     0,  127, offset); break;
	case 4: Adjust2(ti, d_effects[ti].d_delayFeedback, 0,  127, offset); break;

	case 7: Adjust2(ti, d_effects[ti].d_reduce,        0,  127, offset); break;
	default: break; }}


const std::string CXLUnit::GetMixParameterName(int ti, int pi) {
	switch (pi) {
	case 0: return "dis";
	case 1: return "vol";
	case 2: return "pan";
	case 3: return "dly";
	case 4: return "rev";
	default: return ""; }}


int CXLUnit::GetMixParameterValue(int ti, int pi) {
	switch (pi) {
	case 0: return d_mixer.d_channels[ti].d_distortion;
	case 1: return d_mixer.d_channels[ti].d_gain;
	case 2: return d_mixer.d_channels[ti].d_pan;
	case 3: return d_mixer.d_channels[ti].d_send1;
	case 4: return d_mixer.d_channels[ti].d_send2;
	default: return 0; }}


void CXLUnit::AdjustMixParameter(int ti, int pi, int offset) {
	switch (pi) {
	case 0: Adjust2(ti, d_mixer.d_channels[ti].d_distortion, 0, 127, offset); break;
	case 1: Adjust2(ti, d_mixer.d_channels[ti].d_gain, 0, 127, offset); break;
	case 2: Adjust2(ti, d_mixer.d_channels[ti].d_pan, -64, 63, offset); break;
	case 3: Adjust2(ti, d_mixer.d_channels[ti].d_send1, 0, 127, offset); break;
	case 4: Adjust2(ti, d_mixer.d_channels[ti].d_send2, 0, 127, offset); break;
	default: break; }}


const std::string& CXLUnit::GetWaveName(int waveId) {
	return d_waveTable.Get(waveId).d_descr; }


void CXLUnit::DecrementKit() {
	if (d_kitNum > 0) {
		d_kitNum--;
		LoadKit(); }}


void CXLUnit::IncrementKit() {
	if (d_kitNum < kMaxKitNum) {
		d_kitNum++;
		LoadKit(); }}


void CXLUnit::SwitchKit(int n) {
	d_kitNum = n;
	LoadKit(); }


void CXLUnit::SaveKit() {
	const auto& path = MakeKitPath(d_kitNum);
	auto fd = std::ofstream(path.c_str());
	fd << "Name: " << d_kitName << "\n";
	for (int ti=0; ti<kNumVoices; ti++) {
		fd << "Voice #" << ti << "\n";

		for (int pi=0; pi<8; pi++) {
			const auto paramName = GetVoiceParameterName(ti, pi);
			if (paramName == "wav") {
				auto& wave = d_waveTable.Get(d_voices[ti].d_waveId);
				if (wave.d_loaded) {
					fd << "  voice.wav name=" << wave.d_descr << "\n"; }
				else {
					fd << "  voice.wav none\n"; }}
			else {
				const auto paramValue = GetVoiceParameterValue(ti, pi);
				if (!paramName.empty()) {
					fd << fmt::sprintf("  voice.%s %d", paramName, paramValue) << "\n"; }}}

		for (int pi=0; pi<8; pi++) {
			const auto paramName = GetEffectParameterName(ti, pi);
			const auto paramValue = GetEffectParameterValue(ti, pi);
			if (!paramName.empty()) {
				fd << fmt::sprintf("  effect.%s %d", paramName, paramValue) << "\n"; }}

		for (int pi=0; pi<8; pi++) {
			const auto paramName = GetMixParameterName(ti, pi);
			const auto paramValue = GetMixParameterValue(ti, pi);
			if (!paramName.empty()) {
				fd << fmt::sprintf("  mix.%s %d", paramName, paramValue) << "\n"; }}}

	Log::GetInstance().info(fmt::sprintf("saved %s", path)); }


void CXLUnit::InitializeKit() {
	d_kitName = "new kit";
	for (int i=0; i<kNumVoices; i++) {
		d_voices[i].Initialize();
		d_effects[i].Initialize();
		d_mixer.d_channels[i].Initialize(); }}


void CXLUnit::LoadKit() {
	using rclt::ConsumePrefix;
	const auto& path = MakeKitPath(d_kitNum);
	try {
		auto fd = std::ifstream(path.c_str());
		InitializeKit();
		int vid = 0;
		if (fd.good()) {
			std::string line;
			while (getline(fd, line)) {
				if (ConsumePrefix(line, "Name: ")) {
					d_kitName = line; }
				else if (ConsumePrefix(line, "Voice #")) {
					vid = stoi(line); }
				else if (ConsumePrefix(line, "  voice.wav ")) {
					if (line == "none") {
						d_voices[vid].d_waveId = 0; }
					else if (ConsumePrefix(line, "name=")) {
						int waveId = d_waveTable.FindByName(line);
						if (waveId == 0) {
							auto msg = fmt::sprintf("waveTable entry with name \"%s\" not found", line);
							Log::GetInstance().info(msg); }
						d_voices[vid].d_waveId = waveId; }
					else {
						auto msg = fmt::sprintf("invalid wave reference \"%s\"", line);
						Log::GetInstance().info(msg);
						d_voices[vid].d_waveId = 0; }}
				else if (ConsumePrefix(line, "  voice.atk ")) { d_voices[vid].d_attackPct = stoi(line); }
				else if (ConsumePrefix(line, "  voice.dcy ")) { d_voices[vid].d_decayPct = stoi(line); }
				else if (ConsumePrefix(line, "  voice.tun ")) { d_voices[vid].d_tuningInNotes = stoi(line); }
				else if (ConsumePrefix(line, "  voice.fin ")) { d_voices[vid].d_tuningInCents = stoi(line); }
				else if (ConsumePrefix(line, "  effect.flt ")) { d_effects[vid].d_lowpassFreq = stoi(line); }
				else if (ConsumePrefix(line, "  effect.rez ")) { d_effects[vid].d_lowpassQ = stoi(line); }
				else if (ConsumePrefix(line, "  effect.dly ")) { d_effects[vid].d_delaySend = stoi(line); }
				else if (ConsumePrefix(line, "  effect.dtm ")) { d_effects[vid].d_delayTime = stoi(line); }
				else if (ConsumePrefix(line, "  effect.dfb ")) { d_effects[vid].d_delayFeedback = stoi(line); }
				else if (ConsumePrefix(line, "  effect.red ")) { d_effects[vid].d_reduce = stoi(line); }
				else if (ConsumePrefix(line, "  mix.dis ")) { d_mixer.d_channels[vid].d_distortion = stoi(line); }
				else if (ConsumePrefix(line, "  mix.vol ")) { d_mixer.d_channels[vid].d_gain = stoi(line); }
				else if (ConsumePrefix(line, "  mix.pan ")) { d_mixer.d_channels[vid].d_pan = stoi(line); }
				else if (ConsumePrefix(line, "  mix.dly ")) { d_mixer.d_channels[vid].d_send1 = stoi(line); }
				else if (ConsumePrefix(line, "  mix.rev ")) { d_mixer.d_channels[vid].d_send2 = stoi(line); }
				else {
					auto msg = fmt::sprintf("unknown kit line \"%s\"", line);
					throw std::runtime_error(msg); }}
			auto msg = fmt::sprintf("kit %d loaded from %s", d_kitNum, path);
			Log::GetInstance().info(msg); }
		else {
			auto msg = fmt::sprintf("kit %d could not be read, using init kit", d_kitNum);
			Log::GetInstance().info(msg); }}
	catch (const std::exception& err) {
		Log::GetInstance().info(err.what());
		throw; }}


void CXLUnit::Render(float* left, float* right, int numSamples) {
	bool stateChanged = d_sequencer.Update();
	if (stateChanged) {
		d_playbackStateChanged.emit(IsPlaying()); }

	const auto tempo = GetTempo();
	for (int i=0; i<kNumVoices; i++) {
		d_voices[i].Update(tempo);
		d_effects[i].Update(tempo); }
	d_mixer.Update(tempo);

	bool gridPositionUpdated = false;
	for (int si = 0; si < numSamples; si++) {
		bool updated = d_sequencer.Process();
		gridPositionUpdated = gridPositionUpdated || updated;

		std::array<float, 2> masterOut;
		std::array<float, kNumVoices> toMixer;
		for (int i=0; i<kNumVoices; i++) {
			float tmp;
			d_voices[i].Process(nullptr, &tmp);
			d_effects[i].Process(&tmp, &(toMixer[i])); }

		d_mixer.Process(toMixer.data(), masterOut.data());
		left[si] = masterOut[0] * kMasterGain;
		right[si] = masterOut[1] * kMasterGain; }

	if (gridPositionUpdated) {
		d_playbackPositionChanged.emit(GetLastPlayedGridPosition()); }}


void CXLUnit::Trigger(int track) {
	d_voices[track].Trigger(48, 1.0, 0); }


void CXLUnit::CommitPattern() {
	const auto& path = MakePatternPath(d_patternNum);
	int patternLength = GetPatternLength();
	auto fd = std::ofstream(path.c_str());
	fd << "Kit: " << d_kitNum << "\n";
	fd << "Length: " << patternLength << "\n";
	fd << "Tempo: " << (GetTempo()/10) << "." << (GetTempo()%10) << "\n";
	for (int ti=0; ti<kNumVoices; ti++) {
		fd << "Track " << ti << ": ";
		for (int pos=0; pos<patternLength; pos++) {
			fd << (GetTrackGridNote(ti, pos) != 0 ? "X" : ".");}
		fd << "\n"; }
	Log::GetInstance().info(fmt::sprintf("saved pattern %d to \"%s\"", d_patternNum, path)); }


void CXLUnit::SwitchPattern(int pid) {
	const auto& path = MakePatternPath(pid);
	d_sequencer.InitializePattern();
	d_patternNum = pid;
	try {
		auto fd = std::ifstream(path.c_str());
		if (fd.good()) {
			std::string line;
			while (getline(fd, line)) {
				if (rclt::ConsumePrefix(line, "Kit: ")) {
					SwitchKit(stoi(line)); }
				else if (rclt::ConsumePrefix(line, "Length: ")) {
					SetPatternLength(stoi(line)); }
				else if (rclt::ConsumePrefix(line, "Tempo: ")) {
					auto segs = rclt::Split(line, '.');
					int tempo = stoi(segs[0])*10 + stoi(segs[1]);
					SetTempo(tempo); }
				else if (rclt::ConsumePrefix(line, "Track ")) {
					// "Track NN: X..X.XX"
					auto segs = rclt::Split(line, ':');
					auto trackId = std::stoi(segs[0]);
					auto grid = rclt::Trim(segs[1]);
					int pos = 0;
					for (auto ch : grid) {
						bool on = (ch == 'X');
						if (on) {
							ToggleTrackGridNote(trackId, pos); }
						pos++; }}}
			auto msg = fmt::sprintf("loaded pattern %d from \"%s\"", pid, path);
			Log::GetInstance().info(msg); }
		else {
			auto msg = fmt::sprintf("pattern %s could not be read, using init pattern", path);
			Log::GetInstance().info(msg); }}
	catch (const std::exception& err) {
		Log::GetInstance().info(err.what());
		throw; }}


}  // namespace cxl
}  // namespace rqdq
