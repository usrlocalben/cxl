#pragma once
#include <string>
#include <vector>

#include "src/ral/ralw/ralw_mpcwave.hxx"

namespace rqdq {
namespace ralw {

class WaveTable {
public:
	WaveTable();
	WaveTable(int slots);

	int FindByName(const std::string& name);

	MPCWave& Get(const int id) {
		if (!(0 <= id && id < d_waves.size())) {
			throw std::runtime_error("invalid wavetable slot id"); }
		return d_waves[id]; }

	const MPCWave& GetConst(const int id) const {
		if (!(0 <= id && id < d_waves.size())) {
			throw std::runtime_error("invalid wavetable slot id"); }
		return d_waves[id]; }

	//-- clients can call one of these, but not both... maybe protect that somehow later XXX
	//MPCWave* AllocWave();
	//int GetNextWaveId();

	//const std::string& GetWaveDescription(const int id) const { MY_ASSERT(id>=0 && id<wavecnt); return d_waves[id].d_descr; }
	//const int GetNumWaves() const { return d_waves.size(); }
	//int SaveWave(const std::string& path, const int id) const { return d_waves[id].saveWav(path); }

private:
	std::vector<MPCWave> d_waves; };


}  // close package namepsace
}  // close enterprise namespace
