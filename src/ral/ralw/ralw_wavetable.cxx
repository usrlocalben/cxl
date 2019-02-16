#include "src/ral/ralw/ralw_wavetable.hxx"

#include "src/ral/ralw/ralw_mpcwave.hxx"

#include <algorithm>
#include <string>
#include <vector>

namespace rqdq {

namespace {

int clamp(int lower, int value, int upper) {
	return std::max(std::min(value, upper), lower); }

}  // close unnamed namespace

namespace ralw {


WaveTable::WaveTable() {
	d_waves.resize(17);
	d_waves[0].d_descr = "(no sample)"; }


WaveTable::WaveTable(const int slots) {
	d_waves.resize(slots);
	d_waves[0].d_descr = "(no sample)"; }


/*
MPCWave& WaveTable::GetNextWave() {
	return &d_waves[GetNextWaveId()];
}


int WaveTable::GetNextWaveId() {
	assert(wavecnt < wavesz);
	return wavecnt++; }
*/


int WaveTable::FindByName(const std::string& name) {
	for (int i=1; i<d_waves.size(); i++) {
		const auto& wave = d_waves[i];
		if (wave.d_loaded && wave.d_descr==name) {
			return i; }}
	return 0; }


/*
XXX new name creation
	if ( name ) {
		this->description = *name;
	} else {
		//XXX this is really weird because it needs the wavetable to do this.
		char new_name[1024];
		int found = 1;
		while ( found ) {
			sprintf( new_name, "Sample %d", next_sample_number++ );
			found = waveTable.searchByName( new_name );
		}
		this->description = new_name;
	}
*/

}  // close package namepsace
}  // close enterprise namespace
