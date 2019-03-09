#include "src/ral/ralw/ralw_mpcwave.hxx"
#include "src/ral/ralw/ralw_wavfile.hxx"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>

namespace rqdq {
namespace ralw {

void MPCWave::Free() {
	if (!d_loaded) {
		throw std::runtime_error("attempted to release unloaded MPCWave instance"); }
	d_loaded = false;
	d_dataLeft.clear();
	d_dataRight.clear();
	d_descr = "(free sample)"; }


MPCWave MPCWave::Load(const int size,
                      const int freq,
                      const int16_t* const r,
                      const int16_t* const l,
                      const bool stereo) {
	MPCWave instance;
	instance.d_freq = freq;
	instance.d_stereo = stereo;

	instance.d_loopMode = LoopMode::Off;
	instance.d_loopBegin = 0;
	instance.d_loopEnd = size;

	instance.SelectAll();
	instance.ResetRegions(1);

	if (stereo) {
		instance.d_dataLeft.assign(l, l+size);
		instance.d_dataRight.assign(r, r+size); }
	else {
		//XXX the caller should probably need to know
		//that 'l' will be used for both l/r
		instance.d_dataLeft.assign(l, l+size);
		instance.d_dataRight.assign(l, l+size); }

	instance.d_loaded = true;
	return instance; }


void MPCWave::Trim() {
	// drops regions outside the selection interval, e.g. photoshop _crop_
	std::vector<int16_t>(d_dataLeft.begin()+d_selectionBegin, d_dataLeft.begin()+d_selectionEnd).swap(d_dataLeft);
	std::vector<int16_t>(d_dataRight.begin()+d_selectionBegin, d_dataRight.begin()+d_selectionEnd).swap(d_dataRight);
	SelectAll();
	ResetRegions(1); }


void MPCWave::SelectAll() {
	d_selectionBegin = 0;
	d_selectionEnd = GetNumSamples(); }


void MPCWave::Normalize() {
	// find max values
	int maxValue = 0;
	for (int i=0; i<d_dataLeft.size(); i++) {
		maxValue = std::max(maxValue, abs(d_dataLeft[i]));
		maxValue = std::max(maxValue, abs(d_dataRight[i])); }

	if (maxValue >= 32767) {
		return; }

	// compute factor and rescale
	const double factor = 32767.0 / maxValue;
	for (int i=0; i<d_dataLeft.size(); i++) {
		d_dataLeft[i] = int(d_dataLeft[i] * factor);
		d_dataRight[i] = int(d_dataRight[i] * factor); }}


void MPCWave::Reverse() {
	// in-place reverse
	int j=d_selectionBegin;
	int k=d_selectionEnd-1;
	if (j>k) {
		std::swap(j, k); }
	while (j < k) {
		std::swap(d_dataLeft[j], d_dataLeft[k]);
		std::swap(d_dataRight[j], d_dataRight[k]);
		j++; k--; }}


void MPCWave::ResetRegions(const int wanted) {
	if (wanted > GetNumSamples()) {
		throw std::runtime_error("sample too small for requested number of regions"); }

	const int regionSize = GetNumSamples() / wanted;

	int idx;
	int d_regionEnd = regionSize;
	d_markers.resize(wanted+1);

	d_markers[0] = 0;
	for (idx=1; idx<wanted; idx++) {
		d_markers[idx] = d_regionEnd;
		d_regionEnd += regionSize; }
	d_markers[idx] = GetNumSamples(); }

/*
void MPCWave::region_clamp_and_shift( const int rnum) {
	int xmin = rnum;  // region number can also be minimum possible Begin pos if all regions were 1 sample long
	int xmax = len - ( GetNumRegions() - rnum - 1);
	d_markers[rnum] = clamp( d_markers[rnum], xmin, xmax);
	for (int i=rnum-1; i>0; i--)
		if (d_markers[i] >= d_markers[i+1])
			d_markers[i] = d_markers[i+1]-1;
	for ( int i=rnum+1; i<=rcnt; i++)
		if ( d_markers[i] <= d_markers[i-1])
			d_markers[i] = d_markers[i-1]+1;
}

int MPCWave::SetRegionBegin(int rnum, int t) {
	rlst[rnum] = sample;
	region_clamp_and_shift(rnum-1);
	return rlst[r-1];
}

int MPCWave::SetRegionEnd(int rnum, int t) {
	rlst[r+1] = sample;
	region_clamp_and_shift( r);
	return rlst[r];
}

int MPCWave::modifyRegionBegin( const int r, const int amt)
{
	rlst[r-1] += amt;
	region_clamp_and_shift( r-1);
	return rlst[r-1];
}

int MPCWave::modifyRegionEnd( const int r, const int amt)
{
	rlst[r] += amt;
	region_clamp_and_shift(r);
	return rlst[r];
}
*/


MPCWave MPCWave::ExtractRegion(int rnum) const {
	if (!(0 <= rnum && rnum <= GetNumRegions())) {
		throw std::runtime_error("attempted to extract invalid region"); }

	const int rBegin = GetRegionBegin(rnum);
	const int rEnd = GetRegionEnd(rnum);
	const int len = rEnd - rBegin;

	std::stringstream newName;
	newName << d_descr << " " << (rnum+1);
	auto wave = MPCWave::Load(len, d_freq, &d_dataLeft[rBegin], &d_dataRight[rBegin], d_stereo);
	wave.d_descr = newName.str();
	return wave; }


MPCWave MPCWave::Load(const std::string& path, const std::string& wavename) {
	MPCWave instance;
	// XXX gsl::owner<FILE*> fd = fopen(path.c_str(), "rb");
	FILE* fd = fopen(path.c_str(), "rb");
	if (fd == nullptr) {
		throw std::runtime_error("fopen read fail"); }

	fseek(fd, 12, SEEK_SET);

	int have_fmt = 0;
	int have_data = 0;

	std::vector<uint8_t> tmp;
	while (feof(fd) == 0) {

		wavchunkhead wch{};
		fread(&wch, 8, 1, fd);

		tmp.resize(wch.chunk_size, 0);
		size_t blocksread = fread(tmp.data(), wch.chunk_size, 1, fd);

		if (blocksread == 0) {
			break; }
		switch (wch.chunk_id) {
			case WAV_CHUNK_FMT : {
				auto* fmt = reinterpret_cast<wc_fmt*>(tmp.data());
//printf("format_tag:%08x\n", fmt->format_tag);
//printf("channels: %d\n", fmt->channels);
//printf("bitdepth: %d\n", fmt->bitdepth);
//printf("samplerate: %d\n", fmt->hz);
				if (fmt->channels > 2 ||
				    fmt->bitdepth != 16 ||
				    fmt->format_tag != 1) {
					break; }
				instance.d_freq = fmt->hz;
				instance.d_stereo = (fmt->channels==2);
//printf("stereo: %c (chans %d)\n", instance.d_stereo?'Y':'N', fmt->channels);
				have_fmt = 1;
			} break;
			case WAV_CHUNK_FACT: {
//printf("--skipping fact chunk--\n");
			} break;
			case WAV_CHUNK_PAD : {
//printf("--skipping pad chunk--\n");
			} break;
 		case WAV_CHUNK_INST : {
//printf("-skipping inst chunk--\n");
			} break;
			case WAV_CHUNK_LIST : {
//printf("--skipping LIST chunk, %d bytes @ %08x---\n", wch.chunk_size, pos);
			}
			case WAV_CHUNK_SMPL : {
				if (have_data != 0) {
					break; }
				auto* smpl = reinterpret_cast<wc_smpl*>(tmp.data());
//printf("--smpl chunk, %d bytes @ %08x---\n", wch.chunk_size, pos);
//printf("manufacturer:  %d\n", smpl->manufacturer);
//printf("product:       %d\n", smpl->product);
//printf("sample_period: %d\n", smpl->sample_period);
//printf("midi_note:     %d\n", smpl->midi_unity_note);
//printf("pitch_fraction:%d\n", smpl->midi_pitch_faction);
//printf("loops:         %d\n", smpl->sample_loops);
				if (smpl->sample_loops > 0) {
					instance.d_loopMode = LoopMode::Cycle;
					instance.d_loopBegin = smpl->loop.start;
					instance.d_loopEnd   = smpl->loop.end;

//printf("loop1 id: %d %08x\n", smpl->loop.id, smpl->loop.id);
//printf("      ty: %d\n", smpl->loop.type);
//printf("      startbyte: %d\n", smpl->loop.start);
//printf("        endbyte: %d\n", smpl->loop.end);
//printf("       fraction: %d %08x\n", smpl->loop.fraction, smpl->loop.fraction);
//printf("         cycles: %d\n", smpl->loop.playcount);
				} else {
					instance.d_loopMode = LoopMode::Off;
				}
			} break;
			case WAV_CHUNK_CNXL : {
				auto *cnxl = reinterpret_cast<wc_cnxl*>(tmp.data());
				instance.d_selectionBegin = cnxl->trim_start;
				instance.d_selectionEnd   = cnxl->trim_end;
			} break;
			case WAV_CHUNK_DATA: {
				if (have_fmt != 1) {
					break; }
				auto* sbuf = reinterpret_cast<int16_t*>(tmp.data());
				int samples = (wch.chunk_size / 2) / (instance.d_stereo?2:1);
//printf("data chunk is %d bytes\n", wch.chunk_size);
//printf("samples to load: %d\n", samples);
				instance.d_loaded = true;
				instance.d_selectionBegin = 0;
				instance.d_selectionEnd = samples-1;
				instance.d_descr = wavename; //up to _MAX_PATH ? XXX
				instance.d_dataLeft.resize(samples);
				instance.d_dataRight.resize(samples);
				for (int si=0; si<samples; si++) {
					if (instance.d_stereo) {
						instance.d_dataLeft[si] = sbuf[si*2+0];
						instance.d_dataRight[si] = sbuf[si*2+1];
					} else {
						instance.d_dataLeft[si] = sbuf[si];
						instance.d_dataRight[si] = sbuf[si];
					}
				}
				have_data = 1;
			} break;
			default : {
				printf("unknown chunk %c%c%c%c !\n"
				       ,(wch.chunk_id>>0)&0xff
				       ,(wch.chunk_id>>8)&0xff
				       ,(wch.chunk_id>>16)&0xff
				       ,(wch.chunk_id>>24)&0xff);
			} break;
		}
	}
	fclose(fd);

	if (!(have_fmt==1 && have_data==1)) {
		throw std::runtime_error("expected one FMT and one DATA"); }

//getch();
	// XXX instance.d_active = false;
	if (instance.d_loopMode == LoopMode::Cycle) {
		if (instance.d_stereo) {
			instance.d_loopBegin /= 4;
			instance.d_loopEnd /= 4; }
		else {
			instance.d_loopBegin /= 2;
			instance.d_loopEnd /= 2; }}
	else {
		instance.d_loopBegin = instance.d_selectionBegin;
		instance.d_loopEnd   = instance.d_selectionEnd; }

	if (instance.d_loopBegin > instance.GetNumSamples()) {
//printf("WARNING - decoded loop info says loop-start > waveform-length!\n");
//printf("press any key...\n");
//_getch();
		instance.d_loopBegin = 0;
		instance.d_loopEnd = instance.GetNumSamples(); }

	instance.ResetRegions(1);
	return instance; }


MPCWave MPCWave::Load(const std::vector<uint8_t>& buf, const std::string& wavename) {
	MPCWave instance;

	int POS = 12;

	int have_fmt = 0;
	int have_data = 0;

	while (POS < buf.size()) {

		const wavchunkhead& wch = *reinterpret_cast<const wavchunkhead*>(&buf[POS]);
		POS += sizeof(wavchunkhead);

		if (buf.size() - POS < wch.chunk_size) {
			// xxx error log?  reached eof unexpectedly
			break;}

		const uint8_t* tmp = &buf[POS];  POS += wch.chunk_size;

		switch (wch.chunk_id) {
			case WAV_CHUNK_FMT : {
				auto* fmt = reinterpret_cast<const wc_fmt*>(tmp);
//printf("format_tag:%08x\n", fmt->format_tag);
//printf("channels: %d\n", fmt->channels);
//printf("bitdepth: %d\n", fmt->bitdepth);
//printf("samplerate: %d\n", fmt->hz);
				if (fmt->channels > 2 ||
				    fmt->bitdepth != 16 ||
				    fmt->format_tag != 1) {
					break; }
				instance.d_freq = fmt->hz;
				instance.d_stereo = (fmt->channels==2);
//printf("stereo: %c (chans %d)\n", instance.d_stereo?'Y':'N', fmt->channels);
				have_fmt = 1;
			} break;
			case WAV_CHUNK_FACT: {
//printf("--skipping fact chunk--\n");
			} break;
			case WAV_CHUNK_PAD : {
//printf("--skipping pad chunk--\n");
			} break;
 		case WAV_CHUNK_INST : {
//printf("-skipping inst chunk--\n");
			} break;
			case WAV_CHUNK_LIST : {
//printf("--skipping LIST chunk, %d bytes @ %08x---\n", wch.chunk_size, pos);
			}
			case WAV_CHUNK_SMPL : {
				if (have_data != 0) {
					break; }
				auto* smpl = reinterpret_cast<const wc_smpl*>(tmp);
//printf("--smpl chunk, %d bytes @ %08x---\n", wch.chunk_size, pos);
//printf("manufacturer:  %d\n", smpl->manufacturer);
//printf("product:       %d\n", smpl->product);
//printf("sample_period: %d\n", smpl->sample_period);
//printf("midi_note:     %d\n", smpl->midi_unity_note);
//printf("pitch_fraction:%d\n", smpl->midi_pitch_faction);
//printf("loops:         %d\n", smpl->sample_loops);
				if (smpl->sample_loops > 0) {
					instance.d_loopMode = LoopMode::Cycle;
					instance.d_loopBegin = smpl->loop.start;
					instance.d_loopEnd   = smpl->loop.end;

//printf("loop1 id: %d %08x\n", smpl->loop.id, smpl->loop.id);
//printf("      ty: %d\n", smpl->loop.type);
//printf("      startbyte: %d\n", smpl->loop.start);
//printf("        endbyte: %d\n", smpl->loop.end);
//printf("       fraction: %d %08x\n", smpl->loop.fraction, smpl->loop.fraction);
//printf("         cycles: %d\n", smpl->loop.playcount);
				} else {
					instance.d_loopMode = LoopMode::Off;
				}
			} break;
			case WAV_CHUNK_CNXL : {
				auto *cnxl = reinterpret_cast<const wc_cnxl*>(tmp);
				instance.d_selectionBegin = cnxl->trim_start;
				instance.d_selectionEnd   = cnxl->trim_end;
			} break;
			case WAV_CHUNK_DATA: {
				if (have_fmt != 1) {
					break; }
				auto* sbuf = reinterpret_cast<const int16_t*>(tmp);
				int samples = (wch.chunk_size / 2) / (instance.d_stereo?2:1);
//printf("data chunk is %d bytes\n", wch.chunk_size);
//printf("samples to load: %d\n", samples);
				instance.d_loaded = true;
				instance.d_selectionBegin = 0;
				instance.d_selectionEnd = samples-1;
				instance.d_descr = wavename; //up to _MAX_PATH ? XXX
				instance.d_dataLeft.resize(samples);
				instance.d_dataRight.resize(samples);
				for (int si=0; si<samples; si++) {
					if (instance.d_stereo) {
						instance.d_dataLeft[si] = sbuf[si*2+0];
						instance.d_dataRight[si] = sbuf[si*2+1];
					} else {
						instance.d_dataLeft[si] = sbuf[si];
						instance.d_dataRight[si] = sbuf[si];
					}
				}
				have_data = 1;
			} break;
			default : {
				printf("unknown chunk %c%c%c%c !\n"
				       ,(wch.chunk_id>>0)&0xff
				       ,(wch.chunk_id>>8)&0xff
				       ,(wch.chunk_id>>16)&0xff
				       ,(wch.chunk_id>>24)&0xff);
			} break;
		}
	}

	if (!(have_fmt==1 && have_data==1)) {
		throw std::runtime_error("expected one FMT and one DATA"); }

//getch();
	// XXX instance.d_active = false;
	if (instance.d_loopMode == LoopMode::Cycle) {
		if (instance.d_stereo) {
			instance.d_loopBegin /= 4;
			instance.d_loopEnd /= 4; }
		else {
			instance.d_loopBegin /= 2;
			instance.d_loopEnd /= 2; }}
	else {
		instance.d_loopBegin = instance.d_selectionBegin;
		instance.d_loopEnd   = instance.d_selectionEnd; }

	if (instance.d_loopBegin > instance.GetNumSamples()) {
//printf("WARNING - decoded loop info says loop-start > waveform-length!\n");
//printf("press any key...\n");
//_getch();
		instance.d_loopBegin = 0;
		instance.d_loopEnd = instance.GetNumSamples(); }

	instance.ResetRegions(1);
	return instance; }

}  // namespace ralw
}  // namespace rqdq
