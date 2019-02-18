#include "src/cxl/cxl_unit.hxx"

#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_sampler.hxx"
#include "src/ral/ralm/ralm_grid_sequencer.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"
#include "src/rcl/rcls/rcls_file.hxx"

#include <utility>
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <vector>


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

	d_gridSequencer.InitializePattern();

	const std::string samplePath = R"(c:\var\lib\cxl\samples)";
	auto files = rcls::fileglob(samplePath + R"(\*.wav)");
	sort(begin(files), end(files));
	int id = 1;
	for (auto& file : files) {
		std::string baseName = file.substr(0, file.size()-4);
		d_waveTable.Get(id) = ralw::MPCWave::Load(samplePath + "\\" + file, baseName, false);
		std::cout << "loaded \"" << baseName << "\"\n";
		id++; }}


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
	if (d_updateFunc) {
		d_updateFunc();}}


int CXLUnit::GetTempo() {
	return d_gridSequencer.GetTempo(); }


// pattern editing
void CXLUnit::ToggleTrackGridNote(int track, int pos) {
	d_gridSequencer.ToggleTrackGridNote(track, pos);
	if (d_updateFunc) {
		d_updateFunc();}}


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
	if      (pi == 0) { Adjust2(d_voices[ti].d_params.cutoff, 0, 127, amt); }
	else if (pi == 1) { Adjust2(d_voices[ti].d_params.resonance, 0, 127, amt); }
	else if (pi == 2) { Adjust2(d_voices[ti].d_params.attackPct, 1, 100, amt); }
	else if (pi == 3) { Adjust2(d_voices[ti].d_params.decayPct, 1, 100, amt); }
	else if (pi == 4) { Adjust2(d_voices[ti].d_params.waveId, 0, 1000, amt); }}


void CXLUnit::Render(float* left, float* right, int numSamples) {
	bool stateChanged = d_gridSequencer.Update();
	if (stateChanged && d_updateFunc) {
		d_updateFunc(); }

	bool gridPositionUpdated = false;
	for (int si = 0; si < numSamples; si++) {
		bool updated = d_gridSequencer.Tick();
		gridPositionUpdated = gridPositionUpdated || updated;
		std::tie(left[si], right[si]) = d_mixer.GetNextSample(); }

	if (gridPositionUpdated && d_updateFunc) {
		d_updateFunc(); }}

}  // namespace cxl
}  // namespace rqdq
