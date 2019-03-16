#pragma once
#include "src/cxl/ui/log/view.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

class LogController {
public:
	LogController(TextKit::MainLoop& loop);

private:
	void onLogWrite();

public:
	LogView d_view;
private:
	TextKit::MainLoop& d_loop; };


}  // namespace cxl
}  // namespace rqdq
