#include "src/ral/ralw/ralw_wavstream.hxx"
#include "src/ral/ralw/ralw_wavfile.hxx"

#include <cstdio>
#include <stdexcept>
#include <string>

namespace rqdq {
namespace ralw {

WavStream::WavStream(const std::string& filename, const int freq, const bool stereo) :d_stereo(stereo) {
	d_fd = fopen(filename.c_str(), "wb");
	if (d_fd == nullptr) {
		throw std::runtime_error("fopen failed"); }

	fwrite("RIFF", 4, 1, d_fd);
	fwrite("XXXX", 4, 1, d_fd); // placeholder for size of WAVE block
	fwrite("WAVE", 4, 1, d_fd);

	wc_fmt wcfmt;
	wavchunkhead wch;
	wch.chunk_id = WAV_CHUNK_FMT;
	wch.chunk_size = sizeof(wcfmt);
	wcfmt.bitdepth      = 16;
	wcfmt.avg_bytes_sec = freq * (stereo?2:1) * 2;
	wcfmt.blockalign    = (stereo?2:1) * 2;
	wcfmt.channels      = stereo ? 2 : 1;
	wcfmt.format_tag    = 1;
	wcfmt.hz            = freq;
	fwrite(&wch, sizeof(wavchunkhead), 1, d_fd);  d_outputSize += sizeof(wavchunkhead);
	fwrite(&wcfmt, sizeof(wcfmt), 1, d_fd);       d_outputSize += sizeof(wcfmt);

	// have to return to this point later to update the final stream length
	d_dataChunkOffset = ftell(d_fd);

    wch.chunk_id = WAV_CHUNK_DATA;
	wch.chunk_size = 0; //(w->len*2) * (w->stereo+1);
	fwrite(&wch, sizeof(wavchunkhead), 1, d_fd);  d_outputSize += sizeof(wavchunkhead); }


//XXX need to validate the #of samples and the length of this buffer....
void WavStream::Write(const int16_t* const buf, const int numSamples) {
	const int bytesPerSample = sizeof(int16_t) * (d_stereo?2:1);
	const int bytesToWrite = numSamples * bytesPerSample;
	fwrite(buf, sizeof(int16_t)*(d_stereo?2:1), numSamples, d_fd);
	d_outputSize += bytesToWrite;
	d_numSamples += numSamples; }


WavStream::~WavStream() {
	wavchunkhead wch;

	fseek(d_fd, d_dataChunkOffset, SEEK_SET);

    wch.chunk_id = WAV_CHUNK_DATA;
	wch.chunk_size = (d_numSamples*sizeof(int16_t)) * (d_stereo?2:1);
	fwrite(&wch, sizeof(wavchunkhead), 1, d_fd);
	fclose(d_fd); }

}  // namespace ralw
}  // namespace rqdq
