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
constexpr float kInitialGain = 1.0f;

};
namespace cxl {


CXLUnit::CXLUnit()
	:d_waveTable(kMaxWaves) {
	d_sequencer.SetTempo(kDefaultTempo);
	d_voices.reserve(kNumVoices);
	for (int i=0; i<kNumVoices; i++) {
		d_voices.emplace_back(d_waveTable);
		d_mixer.AddChannel(d_voices.back(), kInitialGain);
		d_sequencer.AddTrack(d_voices.back()); }

	BeginLoadingWaves(); }


void CXLUnit::BeginLoadingWaves() {
	d_loading = true;
	d_filesToLoad = rcls::FindGlob(config::sampleDir + R"(\*.wav)");
	sort(begin(d_filesToLoad), end(d_filesToLoad));
	d_nextFileId = 0;
	d_nextWaveId = 1;
	d_loaderStateChanged.emit();
	MakeProgressLoadingWaves(); }


void CXLUnit::MakeProgressLoadingWaves() {
	if (d_nextFileId >= d_filesToLoad.size()) {
		d_loading = false;
		d_loaderStateChanged.emit();
		SwitchPattern(0);
		return; }
	auto& reactor = Reactor::GetInstance();
	const auto wavPath = config::sampleDir + "\\" + d_filesToLoad[d_nextFileId];
	reactor.LoadFile(wavPath,
	                 [&](const std::vector<uint8_t>& data) {
	                     this->onWaveIOComplete(data); },
	                 [&](uint32_t err) {
	                     this->onWaveIOError(err);});}


float CXLUnit::GetLoadingProgress() {
	float total = d_filesToLoad.size();
	float done = d_nextFileId;
	return done / total; }


void CXLUnit::onWaveIOComplete(const std::vector<uint8_t>& data) {
	auto& fileName = d_filesToLoad[d_nextFileId];
	auto baseName = fileName.substr(0, fileName.size() - 4);
	d_waveTable.Get(d_nextWaveId) = ralw::MPCWave::Load(data, baseName);
	Log::GetInstance().info(fmt::sprintf("loaded wave %d \"%s\"", d_nextWaveId, baseName));
	d_loaderStateChanged.emit();
	d_nextFileId++;
	d_nextWaveId++;
	MakeProgressLoadingWaves(); }


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


const std::string CXLUnit::GetVoiceParameterName(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	case 0: return "wav";
	case 1: return "atk";
	case 2: return "dcy";
	default: return ""; }}


int CXLUnit::GetVoiceParameterValue(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	case 0: return d_voices[ti].d_params.waveId;
	case 1: return d_voices[ti].d_params.attackPct;
	case 2: return d_voices[ti].d_params.decayPct;
	default: return 0; }}


void CXLUnit::AdjustVoiceParameter(int ti, int pi, int offset) {
	switch (pi) {
	case 0: Adjust2(ti, d_voices[ti].d_params.waveId,        0, 1000, offset); break;
	case 1: Adjust2(ti, d_voices[ti].d_params.attackPct,     0,  100, offset); break;
	case 2: Adjust2(ti, d_voices[ti].d_params.decayPct,      0,  100, offset); break;
	default: break; }}


const std::string CXLUnit::GetEffectParameterName(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	case 0: return "cut";
	case 1: return "rez";
	case 2: return "dly";
	case 3: return "dtm";
	case 4: return "dfb";
	default: return ""; }}


int CXLUnit::GetEffectParameterValue(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	case 0: return d_voices[ti].d_params.cutoff;
	case 1: return d_voices[ti].d_params.resonance;
	case 2: return d_voices[ti].d_params.delaySend;
	case 3: return d_voices[ti].d_params.delayTime;
	case 4: return d_voices[ti].d_params.delayFeedback;
	default: return 0; }}


void CXLUnit::AdjustEffectParameter(int ti, int pi, int offset) {
	switch (pi) {
	case 0: Adjust2(ti, d_voices[ti].d_params.cutoff,        0,  127, offset); break;
	case 1: Adjust2(ti, d_voices[ti].d_params.resonance,     0,  127, offset); break;
	case 2: Adjust2(ti, d_voices[ti].d_params.delaySend,     0,  127, offset); break;
	case 3: Adjust2(ti, d_voices[ti].d_params.delayTime,     0,  127, offset); break;
	case 4: Adjust2(ti, d_voices[ti].d_params.delayFeedback, 0,  127, offset); break;
	default: break; }}


const std::string CXLUnit::GetMixParameterName(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	default: return ""; }}


int CXLUnit::GetMixParameterValue(int ti, int pi) {
	// XXX track index is for future use
	switch (pi) {
	default: return 0; }}


void CXLUnit::AdjustMixParameter(int ti, int pi, int offset) {
	switch (pi) {
	default: break; }}


const std::string& CXLUnit::GetWaveName(int waveId) {
	return d_waveTable.Get(waveId).d_descr; }


void CXLUnit::DecrementKit() {
	if (d_kitNum > 0) {
		d_kitNum--;
		LoadKit(); }}


void CXLUnit::IncrementKit() {
	if (d_kitNum < 99) {
		d_kitNum++;
		LoadKit(); }}


void CXLUnit::SwitchKit(int n) {
	d_kitNum = n;
	LoadKit(); }


void CXLUnit::SaveKit() {
	const auto path = fmt::format("{}\\kit_{}.txt", config::kitDir, d_kitNum);
	auto fd = std::ofstream(path.c_str());
	fd << "Name: " << d_kitName << "\n";
	for (int ti=0; ti<kNumVoices; ti++) {
		fd << "Voice #" << ti << "\n";
		fd << "  cutoff " << d_voices[ti].d_params.cutoff << "\n";
		fd << "  resonance " << d_voices[ti].d_params.resonance << "\n";
		fd << "  attack " << d_voices[ti].d_params.attackPct << "\n";
		fd << "  decay " << d_voices[ti].d_params.decayPct << "\n";

		fd << "  wave ";
		auto& wave = d_waveTable.Get(d_voices[ti].d_params.waveId);
		if (wave.d_loaded) {
			fd << "name=" << wave.d_descr; }
		else {
			fd << "none"; }
		fd << "\n";

		fd << "  delaySend " << d_voices[ti].d_params.delaySend << "\n";
		fd << "  delayTime " << d_voices[ti].d_params.delayTime << "\n";
		fd << "  delayFeedback " << d_voices[ti].d_params.delayFeedback << "\n"; }}


void CXLUnit::InitializeKit() {
	d_kitName = "new kit";
	for (int i=0; i<kNumVoices; i++) {
		d_voices[i].d_params = raldsp::VoiceParameters{}; }}


void CXLUnit::LoadKit() {
	using rclt::ConsumePrefix;
	const auto path = fmt::format("{}\\kit_{}.txt", config::kitDir, d_kitNum);
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
			else if (ConsumePrefix(line, "  cutoff ")) {
				d_voices[vid].d_params.cutoff = stoi(line); }
			else if (ConsumePrefix(line, "  resonance ")) {
				d_voices[vid].d_params.resonance = stoi(line); }
			else if (ConsumePrefix(line, "  attack ")) {
				d_voices[vid].d_params.attackPct = stoi(line); }
			else if (ConsumePrefix(line, "  decay ")) {
				d_voices[vid].d_params.decayPct = stoi(line); }
			else if (ConsumePrefix(line, "  wave ")) {
				if (line == "none") {
					d_voices[vid].d_params.waveId = 0; }
				else if (ConsumePrefix(line, "name=")) {
					d_voices[vid].d_params.waveId = d_waveTable.FindByName(line); }
				else {
					// XXX error? log
					d_voices[vid].d_params.waveId = 0; }}
			else if (ConsumePrefix(line, "  delaySend ")) {
				d_voices[vid].d_params.delaySend = stoi(line); }
			else if (ConsumePrefix(line, "  delayTime ")) {
				d_voices[vid].d_params.delayTime = stoi(line); }
			else if (ConsumePrefix(line, "  delayFeedback ")) {
				d_voices[vid].d_params.delayFeedback = stoi(line); }
			else {
				auto msg = fmt::format("unknown kit line \"{}\"", line);
				throw std::runtime_error(msg); }}}}


void CXLUnit::Render(float* left, float* right, int numSamples) {
	bool stateChanged = d_sequencer.Update();
	if (stateChanged) {
		d_playbackStateChanged.emit(IsPlaying()); }
	d_mixer.Update(GetTempo());

	bool gridPositionUpdated = false;
	for (int si = 0; si < numSamples; si++) {
		bool updated = d_sequencer.Process();
		gridPositionUpdated = gridPositionUpdated || updated;
		std::array<float, 2> samples;
		d_mixer.Process(nullptr, samples.data());
		left[si] = samples[0];
		right[si] = samples[1]; }

	if (gridPositionUpdated) {
		d_playbackPositionChanged.emit(GetLastPlayedGridPosition()); }}


void CXLUnit::Trigger(int track) {
	d_voices[track].Trigger(36, 1.0, 0); }


void CXLUnit::CommitPattern() {
	const auto path = fmt::format("{}\\pattern_{}.txt", config::patternDir, d_patternNum);
	int patternLength = GetPatternLength();
	auto fd = std::ofstream(path.c_str());
	fd << "Kit: " << d_kitNum << "\n";
	fd << "Length: " << patternLength << "\n";
	for (int ti=0; ti<kNumVoices; ti++) {
		fd << "Track " << ti << ": ";
		for (int pos=0; pos<patternLength; pos++) {
			fd << (GetTrackGridNote(ti, pos) != 0 ? "X" : ".");}
		fd << "\n"; }}


void CXLUnit::SwitchPattern(int pid) {
	const auto path = fmt::format("{}\\pattern_{}.txt", config::patternDir, pid);
	d_sequencer.InitializePattern();
	d_patternNum = pid;
	auto fd = std::ifstream(path.c_str());
	if (fd.good()) {
		std::string line;
		while (getline(fd, line)) {
			if (rclt::ConsumePrefix(line, "Kit: ")) {
				SwitchKit(stoi(line)); }
			else if (rclt::ConsumePrefix(line, "Length: ")) {
				SetPatternLength(stoi(line)); }
			else if (rclt::ConsumePrefix(line, "Track ")) {
				// "Track NN: X..X.XX"
				auto segs = rclt::Explode(line, ':');
				auto trackId = std::stoi(segs[0]);
				auto grid = rclt::Trim(segs[1]);
				int pos = 0;
				for (auto ch : grid) {
					bool on = (ch == 'X');
					if (on) {
						ToggleTrackGridNote(trackId, pos); }
					pos++; }}}}}

}  // namespace cxl
}  // namespace rqdq
