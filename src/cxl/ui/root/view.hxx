#pragma once
#include <deque>
#include <memory>
#include <string>
#include <utility>

#include "src/cxl/ui/host/view.hxx"
#include "src/cxl/ui/log/view.hxx"
#include "src/cxl/ui/pattern/view.hxx"
#include "src/cxl/ui/splash/view.hxx"
#include "src/cxl/unit.hxx"
#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/textkit/widget.hxx"

namespace rqdq {
namespace cxl {

class RootView : public TextKit::Widget {
public:
	RootView(const CXLUnit& /*unit*/, HostView& /*hostView*/, PatternView& /*patternView*/, LogView& /*logView*/, const int& mode);

	// Widget impl
	const rcls::TextCanvas& Draw(int /*width*/, int /*height*/) override;
	std::pair<int, int> Pack(int /*w*/, int /*h*/) override;
	int GetType() override;

private:
	const rcls::TextCanvas& DrawHeader(int width);
	const rcls::TextCanvas& DrawKeyHistory();
	const rcls::TextCanvas& DrawTransportIndicator(int width);

private:
	const CXLUnit& d_unit;
	HostView& d_hostView;
	PatternView& d_patternView;
	LogView& d_logView;
	const int& d_mode;

public:
	std::shared_ptr<TextKit::Widget> d_loading;
	std::shared_ptr<TextKit::Widget> d_splash;

private:
	rcls::TextCanvas d_canvas; };


}  // namespace cxl
}  // namespace rqdq
