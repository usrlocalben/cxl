#pragma once
#include <string>
#include <vector>

namespace rqdq {
namespace ralw {

enum LoopMode {
	Off,
	Cycle,
	Reverse, };

/**
 * PCM Wave with in-place editing methods based on
 * AKAI MPC & S-series samplers
 */
class MPCWave {
public:
	bool d_loaded = false;
	int d_refCnt = 0;
	std::string d_descr;
	int d_freq;
	bool d_stereo;
	std::vector<int16_t> d_dataLeft;
	std::vector<int16_t> d_dataRight;
	LoopMode d_loopMode;
	int d_loopBegin;
	int d_loopEnd;
public:
	int GetNumSamples() const { return d_dataLeft.size(); }

	// seletion editing
public:
	int d_selectionBegin;
	int d_selectionEnd;
public:
	void Trim();
	void Normalize();
	void Reverse();
	void SelectAll();

	// multi-region split
private:
	std::vector<int> d_markers;
	//void region_clamp_and_shift(const int rnum);
public:
	void ResetRegions(int wanted);
	int GetNumRegions() const { return d_markers.size() - 1; }
	int GetRegionBegin(int rnum) const { return d_markers[rnum]; }
	int GetRegionEnd(int rnum) const { return d_markers[rnum+1]; }
	// int setRegionBegin(int r, int sample );
	// int setRegionEnd(int r, int sample );
	//int MoveRegionBegin(int rnum, int amt );
	//int MoveRegionEnd(int rnum, int amt );
	MPCWave ExtractRegion(int rnum) const;
	// XXX void regionSplit( class WaveTable& wt ) const;

	std::pair<float, float> Sample(double t) const {
		int idx = int(t);  // XXX long long?
		double fract = t - idx;

		float vl, vr;
		vl = (d_dataLeft[idx]  + fract*(d_dataLeft[idx+1] -d_dataLeft[idx] )) / 32767.0;
		vr = (d_dataRight[idx] + fract*(d_dataRight[idx+1]-d_dataRight[idx])) / 32767.0;
		return {vl, vr};}


	// io
	//int Save(const std::string& path) const;

	static MPCWave Load(const std::string& path, const std::string& wavename);
	static MPCWave Load(const std::vector<uint8_t>& buf, const std::string& wavename);
	static MPCWave Load(int size, int freq, const int16_t* l, const int16_t* r, bool stereo);

	void Free();
	void Release() { d_refCnt--; }
	void AddReference() { d_refCnt++; } };


}  // close package namespace
}  // close enterprise namespace
