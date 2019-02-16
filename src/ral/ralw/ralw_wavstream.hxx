#pragma once
#include <cstdio>

namespace rqdq {
namespace ralw {

class WavStream {
public:
	WavStream(const string& filename, int freq, bool stereo);
	~WavStream();

	void Write(const int16_t* buf, int numSamples);
	
private:
	//void flush();

private:
	FILE *d_fd;
	int d_outputSize;
	int d_dataChunkOffset;
	int d_numSamples;
	bool d_stereo; };

}  // close package namespace
}  // close enterprise namespace
