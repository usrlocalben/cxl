#pragma once
#include <utility>

namespace rqdq {
namespace raldsp {


class IDSPOutput {
public:
	virtual std::pair<float, float> GetNextSample() = 0;
};


}  // close package namespace
}  // close enterprise namespace
