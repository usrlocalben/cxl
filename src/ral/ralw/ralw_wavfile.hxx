#pragma once
#include <cstdint>

namespace rqdq {
namespace ralw {

struct wc_fmt {
	int16_t format_tag;
	uint16_t channels;
	uint32_t hz;
	uint32_t avg_bytes_sec;
	uint16_t blockalign;
	uint16_t bitdepth; };

struct wc_smplloop {
	int32_t id;
	int32_t type;
	int32_t start;
	int32_t end;
	int32_t fraction;
	int32_t playcount; };

struct wc_smpl {
	int32_t manufacturer;
	int32_t product;
	int32_t sample_period;
    int32_t midi_unity_note;
	int32_t midi_pitch_faction;
	int32_t smpte_format;
	int32_t smpte_offset;
	int32_t sample_loops;
	int32_t sampler_data;
	wc_smplloop loop; };

struct wc_cnxl {
	int32_t trim_start;
	int32_t trim_end; };

constexpr uint32_t WAV_CHUNK_FMT  = 0x20746d66; // 'fmt '
constexpr uint32_t WAV_CHUNK_FACT = 0x74636166;
constexpr uint32_t WAV_CHUNK_SMPL = 0x6c706d73; // smpl
constexpr uint32_t WAV_CHUNK_LOOP = 0x706f6f6c;
constexpr uint32_t WAV_CHUNK_DATA = 0x61746164;
constexpr uint32_t WAV_CHUNK_LIST = 0x5453494c; // LIST
constexpr uint32_t WAV_CHUNK_CNXL = 0x6c786e6c; // cnxl
constexpr uint32_t WAV_CHUNK_PAD  = 0x20444150; // 'PAD '
constexpr uint32_t WAV_CHUNK_INST = 0x74736e69; // 'inst'

struct wavchunkhead {
	uint32_t chunk_id;
	uint32_t chunk_size; };


}  // close package namespace
}  // close enterprise namespace
