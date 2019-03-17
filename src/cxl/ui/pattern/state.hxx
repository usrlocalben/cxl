#pragma once

#include <array>
#include <optional>

namespace rqdq {
namespace cxl {

struct EditorState {
	int knobDir{0};
	int curTrack{0};
	int curGridPage{0};
	int curVoicePage{0};
	std::optional<int> curParam;
	bool isRecording{false}; };


}  // namespace cxl
}  // namespace rqdq
