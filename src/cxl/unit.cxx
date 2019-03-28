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
#include "src/cxl/sequencer.hxx"
#include "src/cxl/log.hxx"
#include "src/ral/raldsp/raldsp_mixer.hxx"
#include "src/ral/raldsp/raldsp_sampler.hxx"
#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavetable.hxx"
#include "src/rcl/rclmt/rclmt_reactor_file.hxx"
#include "src/rcl/rclmt/rclmt_signal.hxx"
#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

#include <fmt/printf.h>

namespace rqdq {
namespace {

constexpr int kNumVoices{16};
constexpr int kMaxWaves{1024};
constexpr int kDefaultTempo{1200};
constexpr int kDefaultSwing{50};
constexpr float kMasterGain{0.666F};
constexpr int kMaxKitNum{99};


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
	impl() :waveTable_(kMaxWaves) {
		sequencer_.SetTempo(kDefaultTempo);
		sequencer_.SetSwing(kDefaultSwing);
		voices_.reserve(kNumVoices);
		effects_.reserve(kNumVoices);
		for (int i=0; i<kNumVoices; i++) {
			voices_.emplace_back(waveTable_);
			effects_.emplace_back();
			mixer_.AddChannel();

			std::optional<int> muteGroupId;
			if (i == 8 || i == 9) {
				muteGroupId = 1; }  // CH & OH are in a fixed mute-group

			sequencer_.AddTrack(voices_.back(), muteGroupId); }
		BeginLoadingWaves(); }

	// transport controls
	void Play() {
		sequencer_.Play(); }
	void Stop() {
		sequencer_.Stop(); }
	bool IsPlaying() const {
		return sequencer_.IsPlaying(); }
	int GetPlayingNoteIndex() const {
		return sequencer_.GetPlayingNoteIndex(); }
	void SetTempo(int value) {
		sequencer_.SetTempo(value);
		tempoChanged_.Emit(value); }
	int GetTempo() const {
		return sequencer_.GetTempo(); }
	void SetSwing(int pct) {
		sequencer_.SetSwing(pct); }
	int GetSwing() const {
		return sequencer_.GetSwing(); }

	// pattern editing
	void ToggleTrackGridNote(int track, int pos) {
		sequencer_.ToggleTrackGridNote(track, pos);
		patternDataChanged_.Emit(track); }

	int GetTrackGridNote(int track, int pos) const {
		return sequencer_.GetTrackGridNote(track, pos); }

	void SetTrackGridNote(int track, int pos, int note) {
		return sequencer_.SetTrackGridNote(track, pos, note); }

	void SwitchPattern(int pid) {
		auto path = MakePatternPath(pid);
		sequencer_.InitializePattern();
		patternNum_ = pid;
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
		auto path = MakePatternPath(patternNum_);
		int patternLength = GetPatternLength();
		auto fd = std::ofstream(path);
		fd << "Kit: " << kitNum_ << "\n";
		fd << "Length: " << patternLength << "\n";
		fd << "Tempo: " << (GetTempo()/10) << "." << (GetTempo()%10) << "\n";
		for (int ti=0; ti<kNumVoices; ti++) {
			fd << "Track " << ti << ": ";
			for (int pos=0; pos<patternLength; pos++) {
				fd << (GetTrackGridNote(ti, pos) != 0 ? "X" : ".");}
			fd << "\n"; }
		Log::GetInstance().info(fmt::sprintf("saved pattern %d to \"%s\"", patternNum_, path)); }

	int GetCurrentPatternNumber() const {
		return patternNum_; }

	int GetPatternLength() const {
		return sequencer_.GetPatternLength(); }

	void SetPatternLength(int value) {
		sequencer_.SetPatternLength(value); }

	bool IsTrackMuted(int ti) const {
		return sequencer_.IsTrackMuted(ti); }

	void ToggleTrackMute(int ti) {
		sequencer_.ToggleTrackMute(ti);
		patternDataChanged_.Emit(ti); }

	// sampler voices
	void InitializeKit() {
		kitName_ = "new kit";
		for (int i=0; i<kNumVoices; i++) {
			voices_[i].Initialize();
			effects_[i].Initialize();
			mixer_[i].Initialize(); }}

	void IncrementKit() {
		if (kitNum_ < kMaxKitNum) {
			kitNum_++;
			LoadKit(); }}

	void DecrementKit() {
		if (kitNum_ > 0) {
			kitNum_--;
			LoadKit(); }}

	void SaveKit() {
		auto path = MakeKitPath(kitNum_);
		auto fd = std::ofstream(path);
		fd << "Name: " << kitName_ << "\n";
		for (int ti=0; ti<kNumVoices; ti++) {
			fd << "Voice #" << ti << "\n";

			for (int pi=0; pi<8; pi++) {
				const auto paramName = GetVoiceParameterName(ti, pi);
				if (paramName == "wav") {
					auto& wave = waveTable_.Get(voices_[ti].params_.waveId);
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
		auto path = MakeKitPath(kitNum_);
		try {
			auto fd = std::ifstream(path);
			InitializeKit();
			int vid = 0;
			if (fd.good()) {
				std::string line;
				while (getline(fd, line)) {
					if (ConsumePrefix(line, "Name: ")) {
						kitName_ = line; }
					else if (ConsumePrefix(line, "Voice #")) {
						vid = stoi(line); }
					else if (ConsumePrefix(line, "  voice.wav ")) {
						if (line == "none") {
							voices_[vid].params_.waveId = 0; }
						else if (ConsumePrefix(line, "name=")) {
							int waveId = waveTable_.FindByName(line);
							if (waveId == 0) {
								auto msg = fmt::sprintf("waveTable entry with name \"%s\" not found", line);
								Log::GetInstance().info(msg); }
							voices_[vid].params_.waveId = waveId; }
						else {
							auto msg = fmt::sprintf("invalid wave reference \"%s\"", line);
							Log::GetInstance().info(msg);
							voices_[vid].params_.waveId = 0; }}
					else if (ConsumePrefix(line, "  voice.atk ")) { voices_[vid].params_.attackTime = stoi(line); }
					else if (ConsumePrefix(line, "  voice.dcy ")) { voices_[vid].params_.decayTime = stoi(line); }
					else if (ConsumePrefix(line, "  voice.tun ")) { voices_[vid].params_.tuningInNotes = stoi(line); }
					else if (ConsumePrefix(line, "  voice.fin ")) { voices_[vid].params_.tuningInCents = stoi(line); }
					else if (ConsumePrefix(line, "  effect.flt ")) { effects_[vid].params_.lowpassFreq = stoi(line); }
					else if (ConsumePrefix(line, "  effect.rez ")) { effects_[vid].params_.lowpassQ = stoi(line); }
					else if (ConsumePrefix(line, "  effect.dly ")) { effects_[vid].params_.delaySend = stoi(line); }
					else if (ConsumePrefix(line, "  effect.dtm ")) { effects_[vid].params_.delayTime = stoi(line); }
					else if (ConsumePrefix(line, "  effect.dfb ")) { effects_[vid].params_.delayFeedback = stoi(line); }
					else if (ConsumePrefix(line, "  effect.red ")) { effects_[vid].params_.reduce = stoi(line); }
					else if (ConsumePrefix(line, "  effect.eqf ")) { effects_[vid].params_.eqCenter = stoi(line); }
					else if (ConsumePrefix(line, "  effect.eqg ")) { effects_[vid].params_.eqGain = stoi(line); }
					else if (ConsumePrefix(line, "  mix.dis ")) { mixer_[vid].params_.distortion = stoi(line); }
					else if (ConsumePrefix(line, "  mix.vol ")) { mixer_[vid].params_.gain = stoi(line); }
					else if (ConsumePrefix(line, "  mix.pan ")) { mixer_[vid].params_.pan = stoi(line); }
					else if (ConsumePrefix(line, "  mix.dly ")) { mixer_[vid].params_.send1 = stoi(line); }
					else if (ConsumePrefix(line, "  mix.rev ")) { mixer_[vid].params_.send2 = stoi(line); }
					else {
						auto msg = fmt::sprintf("unknown kit line \"%s\"", line);
						throw std::runtime_error(msg); }}
				auto msg = fmt::sprintf("kit %d loaded from %s", kitNum_, path);
				Log::GetInstance().info(msg); }
			else {
				auto msg = fmt::sprintf("kit %d could not be read, using init kit", kitNum_);
				Log::GetInstance().info(msg); }}
		catch (const std::exception& err) {
			Log::GetInstance().info(err.what());
			throw; }}

	void SwitchKit(int n) {
		kitNum_ = n;
		LoadKit(); }

	int GetCurrentKitNumber() const {
		return kitNum_; }

	std::string_view GetCurrentKitName() const {
		return kitName_; }

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
		case 0: return voices_[ti].params_.waveId;
		case 1: return voices_[ti].params_.attackTime;
		case 2: return voices_[ti].params_.decayTime;

		case 4: return voices_[ti].params_.tuningInNotes;
		case 5: return voices_[ti].params_.tuningInCents;
		default: return 0; }}
	void AdjustVoiceParameter(int ti, int pi, int offset) {
		switch (pi) {
		case 0: Adjust2(ti, voices_[ti].params_.waveId,          0, 1000, offset); break;
		case 1: Adjust2(ti, voices_[ti].params_.attackTime,      0,  127, offset); break;
		case 2: Adjust2(ti, voices_[ti].params_.decayTime,       0,  127, offset); break;

		case 4: Adjust2(ti, voices_[ti].params_.tuningInNotes, -64,   63, offset); break;
		case 5: Adjust2(ti, voices_[ti].params_.tuningInCents,-100,  100, offset); break;
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
		case 0: return effects_[ti].params_.lowpassFreq;
		case 1: return effects_[ti].params_.lowpassQ;
		case 2: return effects_[ti].params_.eqCenter;
		case 3: return effects_[ti].params_.eqGain;
		case 4: return effects_[ti].params_.delaySend;
		case 5: return effects_[ti].params_.delayTime;
		case 6: return effects_[ti].params_.delayFeedback;
		case 7: return effects_[ti].params_.reduce;
		default: return 0; }}
	void AdjustEffectParameter(int ti, int pi, int offset) {
		switch (pi) {
		case 0: Adjust2(ti, effects_[ti].params_.lowpassFreq,   0,  127, offset); break;
		case 1: Adjust2(ti, effects_[ti].params_.lowpassQ,      0,  127, offset); break;
		case 2: Adjust2(ti, effects_[ti].params_.eqCenter,      0,  127, offset); break;
		case 3: Adjust2(ti, effects_[ti].params_.eqGain,      -64,   64, offset); break;
		case 4: Adjust2(ti, effects_[ti].params_.delaySend,     0,  127, offset); break;
		case 5: Adjust2(ti, effects_[ti].params_.delayTime,     0,  127, offset); break;
		case 6: Adjust2(ti, effects_[ti].params_.delayFeedback, 0,  127, offset); break;
		case 7: Adjust2(ti, effects_[ti].params_.reduce,        0,  127, offset); break;
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
		case 0: return mixer_[ti].params_.distortion;
		case 1: return mixer_[ti].params_.gain;
		case 2: return mixer_[ti].params_.pan;
		case 3: return mixer_[ti].params_.send1;
		case 4: return mixer_[ti].params_.send2;
		default: return 0; }}
	void AdjustMixParameter(int ti, int pi, int offset) {
		switch (pi) {
		case 0: Adjust2(ti, mixer_[ti].params_.distortion, 0, 127, offset); break;
		case 1: Adjust2(ti, mixer_[ti].params_.gain, 0, 127, offset); break;
		case 2: Adjust2(ti, mixer_[ti].params_.pan, -64, 63, offset); break;
		case 3: Adjust2(ti, mixer_[ti].params_.send1, 0, 127, offset); break;
		case 4: Adjust2(ti, mixer_[ti].params_.send2, 0, 127, offset); break;
		default: break; }}

	std::string_view GetWaveName(int waveId) const {
		return waveTable_.Get(waveId).d_descr; }

	void Trigger(int track) {
		voices_[track].Trigger(48, 1.0, 0); }

    // audio render
    void Render(float* left, float* right, int numSamples) {
		bool stateChanged = sequencer_.Update();
		if (stateChanged) {
			playbackStateChanged_.Emit(IsPlaying()); }

		const auto tempo = GetTempo();
		for (int i=0; i<kNumVoices; i++) {
			voices_[i].Update(tempo);
			effects_[i].Update(tempo); }
		mixer_.Update(tempo);

		bool gridPositionUpdated = false;
		for (int si = 0; si < numSamples; si++) {
			bool updated = sequencer_.Process();
			gridPositionUpdated = gridPositionUpdated || updated;

			std::array<float, 2> masterOut;
			std::array<float, kNumVoices> toMixer;
			for (int i=0; i<kNumVoices; i++) {
				float tmp;
				voices_[i].Process(nullptr, &tmp);
				effects_[i].Process(&tmp, &(toMixer[i])); }

			mixer_.Process(toMixer.data(), masterOut.data());
			left[si] = masterOut[0] * kMasterGain;
			right[si] = masterOut[1] * kMasterGain; }

		if (gridPositionUpdated) {
			playbackPositionChanged_.Emit(GetPlayingNoteIndex()); }}

private:
	template<typename T>
	void Adjust2(int id, T& slot, T lower, T upper, T amt) {
		T oldValue = slot;
		T newValue = std::clamp(oldValue+amt, lower, upper);
		if (oldValue != newValue) {
			slot = newValue;
			voiceParameterChanged_.Emit(id);}}

public:
	rclmt::Signal<void(int)> voiceParameterChanged_;
	rclmt::Signal<void(int)> tempoChanged_;
	rclmt::Signal<void(int)> patternDataChanged_;
	rclmt::Signal<void(bool)> playbackStateChanged_;
	rclmt::Signal<void(int)> playbackPositionChanged_;

	// BEGIN wave-table loader
private:
	bool loading_{false};
	int nextFileId_{0};
	int nextWaveId_{1};
	std::vector<std::string> filesToLoad_;
public:
	rclmt::Signal<void()> loaderStateChanged_;
	float GetLoadingProgress() const {
		float total = filesToLoad_.size();
		float done = nextFileId_;
		return done / total; }
	std::string_view GetLoadingName() const {
		if (nextFileId_ < filesToLoad_.size()) {
			return filesToLoad_[nextFileId_]; }
		return "";}
	bool IsLoading() const {
        return loading_; }
private:
	void BeginLoadingWaves() {
		loading_ = true;
		filesToLoad_ = rcls::FindGlob(rcls::JoinPath(config::sampleDir, R"(*.wav)"));
		sort(begin(filesToLoad_), end(filesToLoad_));
		nextFileId_ = 0;
		nextWaveId_ = 1;
		loaderStateChanged_.Emit();
		MakeProgressLoadingWaves(); }

	void MakeProgressLoadingWaves() {
		if (nextFileId_ >= filesToLoad_.size()) {
			return; }
		const auto wavPath = rcls::JoinPath(config::sampleDir, filesToLoad_[nextFileId_]);
		auto& lfd = rclmt::LoadFile(wavPath);
		lfd.AddCallbacks([&](std::vector<uint8_t>& data) { this->onWaveIOComplete(data); },
						 [&](uint32_t err) { this->onWaveIOError(err); }); }

	void onWaveIOComplete(const std::vector<uint8_t>& data) {
		const int fileId = nextFileId_++;
		const int waveId = nextWaveId_++;
		MakeProgressLoadingWaves();

		auto& fileName = filesToLoad_[fileId];
		auto baseName = fileName.substr(0, fileName.size() - 4);
		waveTable_.Get(waveId) = ralw::MPCWave::Load(data, baseName);
		// Log::GetInstance().info(fmt::sprintf("loaded wave %d \"%s\"", nextWaveId_, baseName));
		loaderStateChanged_.Emit();

		if (nextFileId_ >= filesToLoad_.size()) {
			onLoadingComplete(); }}

	void onWaveIOError(int error) {
        auto& log = Log::GetInstance();
		auto msg = fmt::sprintf("onWaveIOError %d", error);
        log.info(msg);
		nextFileId_++;
		MakeProgressLoadingWaves(); }

	void onLoadingComplete() {
		loading_ = false;
		loaderStateChanged_.Emit();
		SwitchPattern(0); }
	//  END  wave-table loader

	int kitNum_{0};
	std::string kitName_{"new kit"};
	int patternNum_{0};
	ralw::WaveTable waveTable_;
	raldsp::BasicMixer<CXLChannelStrip> mixer_;
	GridSequencer sequencer_;
	std::vector<raldsp::SingleSampler> voices_;
	std::vector<CXLEffects> effects_; };


CXLUnit::CXLUnit() :impl_(std::make_unique<impl>()) {}
CXLUnit::~CXLUnit() = default;
CXLUnit::CXLUnit(CXLUnit&&) = default;
CXLUnit& CXLUnit::operator=(CXLUnit&&) = default;

bool CXLUnit::IsLoading() const {
	return impl_->IsLoading(); }
float CXLUnit::GetLoadingProgress() const {
	return impl_->GetLoadingProgress(); }
std::string_view CXLUnit::GetLoadingName() const {
	return impl_->GetLoadingName(); }

void CXLUnit::Play() {
	impl_->Play(); }
void CXLUnit::Stop() {
	impl_->Stop(); }
bool CXLUnit::IsPlaying() const {
	return impl_->IsPlaying(); }
void CXLUnit::SetTempo(int value) {
	impl_->SetTempo(value); }
int CXLUnit::GetTempo() const {
	return impl_->GetTempo(); }
void CXLUnit::SetSwing(int pct) {
	impl_->SetSwing(pct); }
int CXLUnit::GetSwing() const {
	return impl_->GetSwing(); }
void CXLUnit::Trigger(int track) {
	impl_->Trigger(track); }

int CXLUnit::GetCurrentKitNumber() const {
	return impl_->GetCurrentKitNumber(); }
int CXLUnit::GetCurrentPatternNumber() const {
	return impl_->GetCurrentPatternNumber(); }

int CXLUnit::ConnectPlaybackPositionChanged(std::function<void(int)> fn) {
	return impl_->playbackPositionChanged_.Connect(std::move(fn)); }
int CXLUnit::ConnectPlaybackStateChanged(std::function<void(bool)> fn) {
	return impl_->playbackStateChanged_.Connect(std::move(fn)); }
int CXLUnit::ConnectLoaderStateChanged(std::function<void()> fn) {
	return impl_->loaderStateChanged_.Connect(std::move(fn)); }
void CXLUnit::DisconnectPlaybackPositionChanged(int id) {
	impl_->playbackPositionChanged_.Disconnect(id); }
void CXLUnit::DisconnectPlaybackStateChanged(int id) {
	impl_->playbackStateChanged_.Disconnect(id); }
void CXLUnit::DisconnectLoaderStateChanged(int id) {
	impl_->loaderStateChanged_.Disconnect(id); }

std::string_view CXLUnit::GetVoiceParameterName(int ti, int pi) const {
	return impl_->GetVoiceParameterName(ti, pi); }
int CXLUnit::GetVoiceParameterValue(int ti, int pi) const {
	return impl_->GetVoiceParameterValue(ti, pi); }
void CXLUnit::AdjustVoiceParameter(int ti, int pi, int offset) {
	impl_->AdjustVoiceParameter(ti, pi, offset); }

std::string_view CXLUnit::GetEffectParameterName(int ti, int pi) const {
	return impl_->GetEffectParameterName(ti, pi); }
int CXLUnit::GetEffectParameterValue(int ti, int pi) const {
	return impl_->GetEffectParameterValue(ti, pi); }
void CXLUnit::AdjustEffectParameter(int ti, int pi, int offset) {
	impl_->AdjustEffectParameter(ti, pi, offset); }

std::string_view CXLUnit::GetMixParameterName(int ti, int pi) const {
	return impl_->GetMixParameterName(ti, pi); }
int CXLUnit::GetMixParameterValue(int ti, int pi) const {
	return impl_->GetMixParameterValue(ti, pi); }
void CXLUnit::AdjustMixParameter(int ti, int pi, int offset) {
	impl_->AdjustMixParameter(ti, pi, offset); }

int CXLUnit::GetPatternLength() const {
	return impl_->GetPatternLength(); }
void CXLUnit::SetPatternLength(int value) {
	impl_->SetPatternLength(value); }
bool CXLUnit::IsTrackMuted(int ti) const {
	return impl_->IsTrackMuted(ti); }
void CXLUnit::ToggleTrackMute(int ti) {
	impl_->ToggleTrackMute(ti); }
void CXLUnit::CommitPattern() {
	impl_->CommitPattern(); }
void CXLUnit::SaveKit() {
	impl_->SaveKit(); }
void CXLUnit::LoadKit() {
	impl_->LoadKit(); }
void CXLUnit::InitializeKit() {
	impl_->InitializeKit(); }
void CXLUnit::IncrementKit() {
	impl_->IncrementKit(); }
void CXLUnit::DecrementKit() {
	impl_->DecrementKit(); }
void CXLUnit::ToggleTrackGridNote(int track, int pos) {
	impl_->ToggleTrackGridNote(track, pos); }
int CXLUnit::GetTrackGridNote(int track, int pos) const {
	return impl_->GetTrackGridNote(track, pos); }
void CXLUnit::SetTrackGridNote(int track, int pos, int note) {
	impl_->SetTrackGridNote(track, pos, note); }
void CXLUnit::SwitchPattern(int pid) {
	impl_->SwitchPattern(pid); }

void CXLUnit::Render(float* left, float* right, int numSamples) {
	impl_->Render(left, right, numSamples); }
int CXLUnit::GetPlayingNoteIndex() const {
	return impl_->GetPlayingNoteIndex(); }

std::string_view CXLUnit::GetWaveName(int waveId) const {
	return impl_->GetWaveName(waveId); }


}  // namespace cxl
}  // namespace rqdq
