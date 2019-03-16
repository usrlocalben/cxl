#pragma once

#include <array>

namespace rqdq {
namespace cxl {

struct EditorState {
	int curTrack = 0;
	int curGridPage = 0;
	int curVoicePage = 0;
	bool isRecording = false; };


}  // namespace cxl
}  // namespace rqdq
