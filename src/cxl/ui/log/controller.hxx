#pragma once
#include "src/cxl/ui/log/view.hxx"
#include "src/textkit/mainloop.hxx"

namespace rqdq {
namespace cxl {

class LogController {
public:
	LogController(TextKit::MainLoop& loop);
	~LogController();
	LogController(const LogController& other) = delete;
	LogController& operator=(const LogController& other) = delete;

private:
	void onLogWrite();

public:
	LogView d_view;
private:
	TextKit::MainLoop& d_loop;
	int d_signalId{-1}; };


}  // namespace cxl
}  // namespace rqdq
