#pragma once

#include <array>
#include <optional>

namespace rqdq {
namespace cxl {

constexpr int SM_NONE = 0;
constexpr int SM_TAP_TEMPO = 1;

struct EditorState {
	int subMode{SM_NONE};
	int knobDir{0};
	int curTrack{0};
	int curGridPage{0};
	int curVoicePage{0};
	std::optional<int> curParam;
	bool isRecording{false}; };


}  // namespace cxl
}  // namespace rqdq
