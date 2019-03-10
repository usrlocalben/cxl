#pragma once
#include <utility>

namespace rqdq {
namespace raldsp {

class IAudioDevice {
public:
	virtual void Update(int tempo) = 0;
	virtual void Process(float* inputs, float* outputs) = 0;
	virtual ~IAudioDevice() = default;
};


}  // close package namespace
}  // close enterprise namespace
