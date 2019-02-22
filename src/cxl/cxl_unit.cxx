#include "src/cxl/cxl_unit.hxx"
#include "src/cxl/cxl_config.hxx"
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


namespace rqdq {
namespace {

constexpr int kNumVoices = 16;

};
namespace cxl {


CXLUnit::CXLUnit()
	:d_waveTable(1024) {
	d_gridSequencer.SetTempo(1200);
	d_voices.reserve(kNumVoices);
	for (int i=0; i<kNumVoices; i++) {
		d_voices.emplace_back(d_waveTable);
		d_mixer.AddChannel(d_voices.back(), 1.0f);
		d_gridSequencer.AddTrack(d_voices.back()); }

	auto files = rcls::fileglob(config::sampleDir + R"(\*.wav)");
	sort(begin(files), end(files));
	int id = 1;
	for (auto& file : files) {
		std::string baseName = file.substr(0, file.size()-4);
		d_waveTable.Get(id) = ralw::MPCWave::Load(config::sampleDir + "\\" + file, baseName, false);
		std::cout << "loaded \"" << baseName << "\"\n";
		id++; }

	SwitchPattern(0); }


// transport controls
void CXLUnit::Play() {
	d_gridSequencer.Play(); }


void CXLUnit::Stop() {
	d_gridSequencer.Stop(); }


bool CXLUnit::IsPlaying() {
	return d_gridSequencer.IsPlaying(); }


int CXLUnit::GetLastPlayedGridPosition() {
	return d_gridSequencer.GetLastPlayedGridPosition(); }


void CXLUnit::SetTempo(int value) {
	d_gridSequencer.SetTempo(value);
	d_tempoChanged.emit(value); }


int CXLUnit::GetTempo() {
	return d_gridSequencer.GetTempo(); }


// pattern editing
void CXLUnit::ToggleTrackGridNote(int track, int pos) {
	d_gridSequencer.ToggleTrackGridNote(track, pos);
	d_patternDataChanged.emit(track); }


int CXLUnit::GetTrackGridNote(int track, int pos) {
	return d_gridSequencer.GetTrackGridNote(track, pos); }


// sampler voices
const std::string CXLUnit::GetVoiceParameterName(int track, int num) {
	if      (num == 0) { return "f.cutoff"; }
	else if (num == 1) { return "f.resonance"; }
	else if (num == 2) { return "attack"; }
	else if (num == 3) { return "decay"; }
	else if (num == 4) { return "wave#"; }
	else if (num == 5) { return "unused"; }
	else if (num == 6) { return "unused"; }
	else if (num == 7) { return "unused"; }
	else {
		throw std::runtime_error("invalid parameter number"); }}


const std::string& CXLUnit::GetWaveName(int waveId) {
	return d_waveTable.Get(waveId).d_descr; }


int CXLUnit::GetVoiceParameterValue(int track, int num) {
	if (num == 0) { return d_voices[track].d_params.cutoff; }
	else if (num == 1) { return d_voices[track].d_params.resonance; }
	else if (num == 2) { return d_voices[track].d_params.attackPct; }
	else if (num == 3) { return d_voices[track].d_params.decayPct; }
	else if (num == 4) { return d_voices[track].d_params.waveId; }
	else { return 0; }}


void CXLUnit::Adjust(int ti, int pi, int amt) {
	if      (pi == 0) { Adjust2(ti, d_voices[ti].d_params.cutoff, 0, 127, amt); }
	else if (pi == 1) { Adjust2(ti, d_voices[ti].d_params.resonance, 0, 127, amt); }
	else if (pi == 2) { Adjust2(ti, d_voices[ti].d_params.attackPct, 0, 100, amt); }
	else if (pi == 3) { Adjust2(ti, d_voices[ti].d_params.decayPct, 0, 100, amt); }
	else if (pi == 4) { Adjust2(ti, d_voices[ti].d_params.waveId, 0, 1000, amt); }}


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
	for (int ti=0; ti<16; ti++) {
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
		fd << "\n"; }}


void CXLUnit::InitializeKit() {
	d_kitName = "new kit";
	for (int i=0; i<16; i++) {
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
			else {
				auto msg = fmt::format("unknown kit line \"{}\"", line);
				throw std::runtime_error(msg); }}}}


void CXLUnit::Render(float* left, float* right, int numSamples) {
	bool stateChanged = d_gridSequencer.Update();
	if (stateChanged) {
		d_playbackStateChanged.emit(IsPlaying()); }

	bool gridPositionUpdated = false;
	for (int si = 0; si < numSamples; si++) {
		bool updated = d_gridSequencer.Tick();
		gridPositionUpdated = gridPositionUpdated || updated;
		std::tie(left[si], right[si]) = d_mixer.GetNextSample(); }

	if (gridPositionUpdated) {
		d_playbackPositionChanged.emit(GetLastPlayedGridPosition()); }}


void CXLUnit::Trigger(int track) {
	d_voices[track].Trigger(36, 1.0, 0); }


void CXLUnit::CommitPattern() {
	const auto path = fmt::format("{}\\pattern_{}.txt", config::patternDir, d_patternNum);
	auto fd = std::ofstream(path.c_str());
	fd << "Kit: " << d_kitNum << "\n";
	for (int ti=0; ti<16; ti++) {
		bool first = true;
		for (int pos=0; pos<16; pos++) {
			if (first) {
				fd << "Track " << ti << ": ";
				first = false; }
			else {
				fd << ","; }
			fd << (GetTrackGridNote(ti, pos) != 0 ? "X" : ".");}
		fd << "\n"; }}


void CXLUnit::SwitchPattern(int pid) {
	const auto path = fmt::format("{}\\pattern_{}.txt", config::patternDir, pid);
	d_gridSequencer.InitializePattern();
	d_patternNum = pid;
	auto fd = std::ifstream(path.c_str());
	if (fd.good()) {
		std::string line;
		while (getline(fd, line)) {
			if (rclt::ConsumePrefix(line, "Kit: ")) {
				SwitchKit(stoi(line)); }
			else if (rclt::ConsumePrefix(line, "Track ")) {
				auto segs = rclt::Explode(line, ':');
				auto trackId = std::stoi(segs[0]);
				segs = rclt::Explode(segs[1], ',');
				int pos = 0;
				for (auto& chunk : segs) {
					bool on = chunk.find('X') != std::string::npos;
					if (on) {
						ToggleTrackGridNote(trackId, pos); }
					pos++; }}}}}

}  // namespace cxl
}  // namespace rqdq
