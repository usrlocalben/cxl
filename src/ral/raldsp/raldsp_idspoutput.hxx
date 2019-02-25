#pragma once
#include <utility>

namespace rqdq {
namespace raldsp {


class IDSPOutput {
public:
	virtual void Update(int tempo) = 0;
	virtual void Process(float* inputs, float* outputs) = 0;
	virtual ~IDSPOutput() = default;
};


}  // close package namespace
}  // close enterprise namespace
