#pragma once
#include "src/cxl/ui/loading/view.hxx"
#include "src/cxl/unit.hxx"

namespace rqdq {
namespace cxl {


class LoadingController {
public:
	LoadingController(CXLUnit& unit);

private:
	void onLoaderStateChange();

private:
	CXLUnit& d_unit;
public:
    LoadingView d_view; };


}  // namespace cxl
}  // namespace rqdq
