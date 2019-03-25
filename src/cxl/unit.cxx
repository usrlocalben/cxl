#include "unit.hxx"

#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "src/cxl/channelstrip.hxx"
#include "src/cxl/config.hxx"
#include "src/cxl/effect.hxx"
#include "src/cxl/log.hxx"
#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_sampler.hxx"
#include "src/ral/ralm/ralm_grid_sequencer.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"
#include "src/rcl/rclmt/rclmt_reactor_file.hxx"
#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

#include <fmt/printf.h>
#include <wink/signal.hpp>

namespace rqdq {
namespace {

constexpr int kNumVoices = 16;
constexpr int kMaxWaves = 1024;
constexpr int kDefaultTempo = 1200;
constexpr int kDefaultSwing = 50;
constexpr float kMasterGain = 0.666F;
constexpr int kMaxKitNum = 99;


const char* MakeKitPath(int n) {
	static std::string out;
	out = rcls::JoinPath(cxl::config::kitDir, fmt::sprintf("%02d.kit", n));
	return out.data(); }


const char* MakePatternPath(int n) {
	static std::string out;
	out = rcls::JoinPath(cxl::config::patternDir, fmt::sprintf("A%02d.pattern", n));
	return out.data(); }


}  // namespace

namespace cxl {


class CXLUnit::impl {
public:
	impl() :d_waveTable(kMaxWaves) {
		d_sequencer.SetTempo(kDefaultTempo);
		d_sequencer.SetSwing(kDefaultSwing);
		d_voices.reserve(kNumVoices);
		d_effects.reserve(kNumVoices);
		for (int i=0; i<kNumVoices; i++) {
			d_voices.emplace_back(d_waveTable);
			d_effects.emplace_back();
			d_mixer.AddChannel();

			std::optional<int> muteGroupId;
			if (i == 8 || i == 9) {
				muteGroupId = 1; }  // CH & OH are in a fixed mute-group

			d_sequencer.AddTrack(d_voices.back(), muteGroupId); }
		BeginLoadingWaves(); }

	// transport controls
	void Play() {
		d_sequencer.Play(); }
	void Stop() {
		d_sequencer.Stop(); }
	bool IsPlaying() const {
		return d_sequencer.IsPlaying(); }
	int GetPlayingNoteIndex() const {
		return d_sequencer.GetPlayingNoteIndex(); }
	void SetTempo(int value) {
		d_sequencer.SetTempo(value);
		d_tempoChanged.emit(value); }
	int GetTempo() const {
		return d_sequencer.GetTempo(); }
	void SetSwing(int pct) {
		d_sequencer.SetSwing(pct); }
	int GetSwing() const {
		return d_sequencer.GetSwing(); }

	// pattern editing
	void ToggleTrackGridNote(int track, int pos) {
		d_sequencer.ToggleTrackGridNote(track, pos);
		d_patternDataChanged.emit(track); }

	int GetTrackGridNote(int track, int pos) const {
		return d_sequencer.GetTrackGridNote(track, pos); }

	void SetTrackGridNote(int track, int pos, int note) {
		return d_sequencer.SetTrackGridNote(track, pos, note); }

	void SwitchPattern(int pid) {
		auto path = MakePatternPath(pid);
		d_sequencer.InitializePattern();
		d_patternNum = pid;
		try {
			auto fd = std::ifstream(path);
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

	void CommitPattern() {
		auto path = MakePatternPath(d_patternNum);
		int patternLength = GetPatternLength();
		auto fd = std::ofstream(path);
		fd << "Kit: " << d_kitNum << "\n";
		fd << "Length: " << patternLength << "\n";
		fd << "Tempo: " << (GetTempo()/10) << "." << (GetTempo()%10) << "\n";
		for (int ti=0; ti<kNumVoices; ti++) {
			fd << "Track " << ti << ": ";
			for (int pos=0; pos<patternLength; pos++) {
				fd << (GetTrackGridNote(ti, pos) != 0 ? "X" : ".");}
			fd << "\n"; }
		Log::GetInstance().info(fmt::sprintf("saved pattern %d to \"%s\"", d_patternNum, path)); }

	int GetCurrentPatternNumber() const {
		return d_patternNum; }

	int GetPatternLength() const {
		return d_sequencer.GetPatternLength(); }

	void SetPatternLength(int value) {
		d_sequencer.SetPatternLength(value); }

	bool IsTrackMuted(int ti) const {
		return d_sequencer.IsTrackMuted(ti); }

	void ToggleTrackMute(int ti) {
		d_sequencer.ToggleTrackMute(ti);
		d_patternDataChanged.emit(ti); }

	// sampler voices
	void InitializeKit() {
		d_kitName = "new kit";
		for (int i=0; i<kNumVoices; i++) {
			d_voices[i].Initialize();
			d_effects[i].Initialize();
			d_mixer[i].Initialize(); }}

	void IncrementKit() {
		if (d_kitNum < kMaxKitNum) {
			d_kitNum++;
			LoadKit(); }}

	void DecrementKit() {
		if (d_kitNum > 0) {
			d_kitNum--;
			LoadKit(); }}

	void SaveKit() {
		auto path = MakeKitPath(d_kitNum);
		auto fd = std::ofstream(path);
		fd << "Name: " << d_kitName << "\n";
		for (int ti=0; ti<kNumVoices; ti++) {
			fd << "Voice #" << ti << "\n";

			for (int pi=0; pi<8; pi++) {
				const auto paramName = GetVoiceParameterName(ti, pi);
				if (paramName == "wav") {
					auto& wave = d_waveTable.Get(d_voices[ti].params.waveId);
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

	void LoadKit() {
		using rclt::ConsumePrefix;
		auto path = MakeKitPath(d_kitNum);
		try {
			auto fd = std::ifstream(path);
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
							d_voices[vid].params.waveId = 0; }
						else if (ConsumePrefix(line, "name=")) {
							int waveId = d_waveTable.FindByName(line);
							if (waveId == 0) {
								auto msg = fmt::sprintf("waveTable entry with name \"%s\" not found", line);
								Log::GetInstance().info(msg); }
							d_voices[vid].params.waveId = waveId; }
						else {
							auto msg = fmt::sprintf("invalid wave reference \"%s\"", line);
							Log::GetInstance().info(msg);
							d_voices[vid].params.waveId = 0; }}
					else if (ConsumePrefix(line, "  voice.atk ")) { d_voices[vid].params.attackTime = stoi(line); }
					else if (ConsumePrefix(line, "  voice.dcy ")) { d_voices[vid].params.decayTime = stoi(line); }
					else if (ConsumePrefix(line, "  voice.tun ")) { d_voices[vid].params.tuningInNotes = stoi(line); }
					else if (ConsumePrefix(line, "  voice.fin ")) { d_voices[vid].params.tuningInCents = stoi(line); }
					else if (ConsumePrefix(line, "  effect.flt ")) { d_effects[vid].d_lowpassFreq = stoi(line); }
					else if (ConsumePrefix(line, "  effect.rez ")) { d_effects[vid].d_lowpassQ = stoi(line); }
					else if (ConsumePrefix(line, "  effect.dly ")) { d_effects[vid].d_delaySend = stoi(line); }
					else if (ConsumePrefix(line, "  effect.dtm ")) { d_effects[vid].d_delayTime = stoi(line); }
					else if (ConsumePrefix(line, "  effect.dfb ")) { d_effects[vid].d_delayFeedback = stoi(line); }
					else if (ConsumePrefix(line, "  effect.red ")) { d_effects[vid].d_reduce = stoi(line); }
					else if (ConsumePrefix(line, "  effect.eqf ")) { d_effects[vid].d_eqCenter = stoi(line); }
					else if (ConsumePrefix(line, "  effect.eqg ")) { d_effects[vid].d_eqGain = stoi(line); }
					else if (ConsumePrefix(line, "  mix.dis ")) { d_mixer[vid].d_distortion = stoi(line); }
					else if (ConsumePrefix(line, "  mix.vol ")) { d_mixer[vid].d_gain = stoi(line); }
					else if (ConsumePrefix(line, "  mix.pan ")) { d_mixer[vid].d_pan = stoi(line); }
					else if (ConsumePrefix(line, "  mix.dly ")) { d_mixer[vid].d_send1 = stoi(line); }
					else if (ConsumePrefix(line, "  mix.rev ")) { d_mixer[vid].d_send2 = stoi(line); }
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

	void SwitchKit(int n) {
		d_kitNum = n;
		LoadKit(); }

	int GetCurrentKitNumber() const {
		return d_kitNum; }

	std::string_view GetCurrentKitName() const {
		return d_kitName; }

	std::string_view GetVoiceParameterName(int ti, int pi) const {
		// XXX track index is for future use
		switch (pi) {
		case 0: return "wav";
		case 1: return "atk";
		case 2: return "dcy";

		case 4: return "tun";
		case 5: return "fin";
		default: return ""; }}
	int GetVoiceParameterValue(int ti, int pi) const {
		// XXX track index is for future use
		switch (pi) {
		case 0: return d_voices[ti].params.waveId;
		case 1: return d_voices[ti].params.attackTime;
		case 2: return d_voices[ti].params.decayTime;

		case 4: return d_voices[ti].params.tuningInNotes;
		case 5: return d_voices[ti].params.tuningInCents;
		default: return 0; }}
	void AdjustVoiceParameter(int ti, int pi, int offset) {
		switch (pi) {
		case 0: Adjust2(ti, d_voices[ti].params.waveId,          0, 1000, offset); break;
		case 1: Adjust2(ti, d_voices[ti].params.attackTime,      0,  127, offset); break;
		case 2: Adjust2(ti, d_voices[ti].params.decayTime,       0,  127, offset); break;

		case 4: Adjust2(ti, d_voices[ti].params.tuningInNotes, -64,   63, offset); break;
		case 5: Adjust2(ti, d_voices[ti].params.tuningInCents,-100,  100, offset); break;
		default: break; }}

	std::string_view GetEffectParameterName(int ti, int pi) const {
		// XXX track index is for future use
		switch (pi) {
		case 0: return "flt";
		case 1: return "rez";
		case 2: return "eqf";
		case 3: return "eqg";
		case 4: return "dly";
		case 5: return "dtm";
		case 6: return "dfb";
		case 7: return "red";
		default: return ""; }}

	int GetEffectParameterValue(int ti, int pi) const {
		// XXX track index is for future use
		switch (pi) {
		case 0: return d_effects[ti].d_lowpassFreq;
		case 1: return d_effects[ti].d_lowpassQ;
		case 2: return d_effects[ti].d_eqCenter;
		case 3: return d_effects[ti].d_eqGain;
		case 4: return d_effects[ti].d_delaySend;
		case 5: return d_effects[ti].d_delayTime;
		case 6: return d_effects[ti].d_delayFeedback;
		case 7: return d_effects[ti].d_reduce;
		default: return 0; }}
	void AdjustEffectParameter(int ti, int pi, int offset) {
		switch (pi) {
		case 0: Adjust2(ti, d_effects[ti].d_lowpassFreq,   0,  127, offset); break;
		case 1: Adjust2(ti, d_effects[ti].d_lowpassQ,      0,  127, offset); break;
		case 2: Adjust2(ti, d_effects[ti].d_eqCenter,      0,  127, offset); break;
		case 3: Adjust2(ti, d_effects[ti].d_eqGain,      -64,   64, offset); break;
		case 4: Adjust2(ti, d_effects[ti].d_delaySend,     0,  127, offset); break;
		case 5: Adjust2(ti, d_effects[ti].d_delayTime,     0,  127, offset); break;
		case 6: Adjust2(ti, d_effects[ti].d_delayFeedback, 0,  127, offset); break;
		case 7: Adjust2(ti, d_effects[ti].d_reduce,        0,  127, offset); break;
		default: break; }}

	std::string_view GetMixParameterName(int ti, int pi) const {
		switch (pi) {
		case 0: return "dis";
		case 1: return "vol";
		case 2: return "pan";
		case 3: return "dly";
		case 4: return "rev";
		default: return ""; }}
	int GetMixParameterValue(int ti, int pi) const {
		switch (pi) {
		case 0: return d_mixer[ti].d_distortion;
		case 1: return d_mixer[ti].d_gain;
		case 2: return d_mixer[ti].d_pan;
		case 3: return d_mixer[ti].d_send1;
		case 4: return d_mixer[ti].d_send2;
		default: return 0; }}
	void AdjustMixParameter(int ti, int pi, int offset) {
		switch (pi) {
		case 0: Adjust2(ti, d_mixer[ti].d_distortion, 0, 127, offset); break;
		case 1: Adjust2(ti, d_mixer[ti].d_gain, 0, 127, offset); break;
		case 2: Adjust2(ti, d_mixer[ti].d_pan, -64, 63, offset); break;
		case 3: Adjust2(ti, d_mixer[ti].d_send1, 0, 127, offset); break;
		case 4: Adjust2(ti, d_mixer[ti].d_send2, 0, 127, offset); break;
		default: break; }}

	std::string_view GetWaveName(int waveId) const {
		return d_waveTable.Get(waveId).d_descr; }

	void Trigger(int track) {
		d_voices[track].Trigger(48, 1.0, 0); }

    // audio render
    void Render(float* left, float* right, int numSamples) {
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
			d_playbackPositionChanged.emit(GetPlayingNoteIndex()); }}

private:
	template<typename T>
	void Adjust2(int id, T& slot, T lower, T upper, T amt) {
		T oldValue = slot;
		T newValue = std::clamp(oldValue+amt, lower, upper);
		if (oldValue != newValue) {
			slot = newValue;
			d_voiceParameterChanged.emit(id);}}

public:
	wink::signal<std::function<void(int)>> d_voiceParameterChanged;
	wink::signal<std::function<void(int)>> d_tempoChanged;
	wink::signal<std::function<void(int)>> d_patternDataChanged;
	wink::signal<std::function<void(bool)>> d_playbackStateChanged;
	wink::signal<std::function<void(int)>> d_playbackPositionChanged;
	wink::signal<std::function<void()>> d_currentPatternChanged;

	// BEGIN wave-table loader
private:
	bool d_loading = false;
	int d_nextFileId = 0;
	int d_nextWaveId = 1;
	std::vector<std::string> d_filesToLoad;
public:
	wink::signal<std::function<void()>> d_loaderStateChanged;
	float GetLoadingProgress() const {
		float total = d_filesToLoad.size();
		float done = d_nextFileId;
		return done / total; }
	std::string_view GetLoadingName() const {
		if (d_nextFileId < d_filesToLoad.size()) {
			return d_filesToLoad[d_nextFileId]; }
		return "";}
	bool IsLoading() const { return d_loading; }
private:
	void BeginLoadingWaves() {
		d_loading = true;
		d_filesToLoad = rcls::FindGlob(rcls::JoinPath(config::sampleDir, R"(*.wav)"));
		sort(begin(d_filesToLoad), end(d_filesToLoad));
		d_nextFileId = 0;
		d_nextWaveId = 1;
		d_loaderStateChanged.emit();
		MakeProgressLoadingWaves(); }

	void MakeProgressLoadingWaves() {
		if (d_nextFileId >= d_filesToLoad.size()) {
			return; }
		const auto wavPath = rcls::JoinPath(config::sampleDir, d_filesToLoad[d_nextFileId]);
		auto& lfd = rclmt::LoadFile(wavPath);
		lfd.AddCallbacks([&](std::vector<uint8_t>& data) { this->onWaveIOComplete(data); },
						 [&](uint32_t err) { this->onWaveIOError(err); }); }

	void onWaveIOComplete(const std::vector<uint8_t>& data) {
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

	void onWaveIOError(int error) {
		d_nextFileId++;
		MakeProgressLoadingWaves(); }

	void onLoadingComplete() {
		d_loading = false;
		d_loaderStateChanged.emit();
		SwitchPattern(0); }
	//  END  wave-table loader

	int d_kitNum = 0;
	std::string d_kitName = "new kit";
	int d_patternNum = 0;
	ralw::WaveTable d_waveTable;
	raldsp::BasicMixer<CXLChannelStrip> d_mixer;
	ralm::GridSequencer d_sequencer;
	std::vector<raldsp::SingleSampler> d_voices;
	std::vector<CXLEffects> d_effects; };


CXLUnit::CXLUnit() :d_impl(std::make_unique<impl>()) {}
CXLUnit::~CXLUnit() = default;
CXLUnit::CXLUnit(CXLUnit&&) = default;
CXLUnit& CXLUnit::operator=(CXLUnit&&) = default;

bool CXLUnit::IsLoading() const {
	return d_impl->IsLoading(); }
float CXLUnit::GetLoadingProgress() const {
	return d_impl->GetLoadingProgress(); }
std::string_view CXLUnit::GetLoadingName() const {
	return d_impl->GetLoadingName(); }

void CXLUnit::Play() {
	d_impl->Play(); }
void CXLUnit::Stop() {
	d_impl->Stop(); }
bool CXLUnit::IsPlaying() const {
	return d_impl->IsPlaying(); }
void CXLUnit::SetTempo(int value) {
	d_impl->SetTempo(value); }
int CXLUnit::GetTempo() const {
	return d_impl->GetTempo(); }
void CXLUnit::SetSwing(int pct) {
	d_impl->SetSwing(pct); }
int CXLUnit::GetSwing() const {
	return d_impl->GetSwing(); }
void CXLUnit::Trigger(int track) {
	d_impl->Trigger(track); }

int CXLUnit::GetCurrentKitNumber() const {
	return d_impl->GetCurrentKitNumber(); }
int CXLUnit::GetCurrentPatternNumber() const {
	return d_impl->GetCurrentPatternNumber(); }

void CXLUnit::ConnectPlaybackPositionChanged(std::function<void(int)> fn) {
	d_impl->d_playbackPositionChanged.connect(std::move(fn)); }
void CXLUnit::ConnectPlaybackStateChanged(std::function<void(bool)> fn) {
	d_impl->d_playbackStateChanged.connect(std::move(fn)); }
void CXLUnit::ConnectLoaderStateChanged(std::function<void()> fn) {
	d_impl->d_loaderStateChanged.connect(std::move(fn)); }

std::string_view CXLUnit::GetVoiceParameterName(int ti, int pi) const {
	return d_impl->GetVoiceParameterName(ti, pi); }
int CXLUnit::GetVoiceParameterValue(int ti, int pi) const {
	return d_impl->GetVoiceParameterValue(ti, pi); }
void CXLUnit::AdjustVoiceParameter(int ti, int pi, int offset) {
	d_impl->AdjustVoiceParameter(ti, pi, offset); }

std::string_view CXLUnit::GetEffectParameterName(int ti, int pi) const {
	return d_impl->GetEffectParameterName(ti, pi); }
int CXLUnit::GetEffectParameterValue(int ti, int pi) const {
	return d_impl->GetEffectParameterValue(ti, pi); }
void CXLUnit::AdjustEffectParameter(int ti, int pi, int offset) {
	d_impl->AdjustEffectParameter(ti, pi, offset); }

std::string_view CXLUnit::GetMixParameterName(int ti, int pi) const {
	return d_impl->GetMixParameterName(ti, pi); }
int CXLUnit::GetMixParameterValue(int ti, int pi) const {
	return d_impl->GetMixParameterValue(ti, pi); }
void CXLUnit::AdjustMixParameter(int ti, int pi, int offset) {
	d_impl->AdjustMixParameter(ti, pi, offset); }

int CXLUnit::GetPatternLength() const {
	return d_impl->GetPatternLength(); }
void CXLUnit::SetPatternLength(int value) {
	d_impl->SetPatternLength(value); }
bool CXLUnit::IsTrackMuted(int ti) const {
	return d_impl->IsTrackMuted(ti); }
void CXLUnit::ToggleTrackMute(int ti) {
	d_impl->ToggleTrackMute(ti); }
void CXLUnit::CommitPattern() {
	d_impl->CommitPattern(); }
void CXLUnit::SaveKit() {
	d_impl->SaveKit(); }
void CXLUnit::LoadKit() {
	d_impl->LoadKit(); }
void CXLUnit::InitializeKit() {
	d_impl->InitializeKit(); }
void CXLUnit::IncrementKit() {
	d_impl->IncrementKit(); }
void CXLUnit::DecrementKit() {
	d_impl->DecrementKit(); }
void CXLUnit::ToggleTrackGridNote(int track, int pos) {
	d_impl->ToggleTrackGridNote(track, pos); }
int CXLUnit::GetTrackGridNote(int track, int pos) const {
	return d_impl->GetTrackGridNote(track, pos); }
void CXLUnit::SetTrackGridNote(int track, int pos, int note) {
	d_impl->SetTrackGridNote(track, pos, note); }
void CXLUnit::SwitchPattern(int pid) {
	d_impl->SwitchPattern(pid); }

void CXLUnit::Render(float* left, float* right, int numSamples) {
	d_impl->Render(left, right, numSamples); }
int CXLUnit::GetPlayingNoteIndex() const {
	return d_impl->GetPlayingNoteIndex(); }

std::string_view CXLUnit::GetWaveName(int waveId) const {
	return d_impl->GetWaveName(waveId); }


}  // namespace cxl
}  // namespace rqdq
