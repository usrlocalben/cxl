#include "src/cxl/ui/loading/controller.hxx"

#include "src/cxl/unit.hxx"

namespace rqdq {
namespace cxl {

LoadingController::LoadingController(CXLUnit& unit) :d_unit(unit), d_view(d_unit) {
	d_unit.d_loaderStateChanged.connect(this, &LoadingController::onLoaderStateChange); }


void LoadingController::onLoaderStateChange() {
	d_view.Invalidate(); }


}  // namespace cxl
}  // namespace rqdq
