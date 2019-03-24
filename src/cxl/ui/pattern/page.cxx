#include "src/cxl/ui/pattern/page.hxx"

#include <stdexcept>
#include <string_view>

#include <fmt/printf.h>

namespace rqdq {
namespace cxl {

std::string_view GetPageParameterName(const CXLUnit& unit, int pageNum, int trackNum, int paramNum) {
	if (pageNum == 0) {
		return unit.GetVoiceParameterName(trackNum, paramNum); }
	if (pageNum == 1) {
		return unit.GetEffectParameterName(trackNum, paramNum); }
	if (pageNum == 2) {
		return unit.GetMixParameterName(trackNum, paramNum); }
	auto msg = fmt::sprintf("invalid parameter page %d", pageNum);
	throw std::runtime_error(msg); }


int GetPageParameterValue(const CXLUnit& unit, int pageNum, int trackNum, int paramNum) {
	if (pageNum == 0) {
		return unit.GetVoiceParameterValue(trackNum, paramNum); }
	if (pageNum == 1) {
		return unit.GetEffectParameterValue(trackNum, paramNum); }
	if (pageNum == 2) {
		return unit.GetMixParameterValue(trackNum, paramNum); }
	auto msg = fmt::sprintf("invalid parameter page %d", pageNum);
	throw std::runtime_error(msg); }


void AdjustPageParameter(CXLUnit& unit, int pageNum, int trackNum, int paramNum, int offset) {
	if (pageNum == 0) {
		unit.AdjustVoiceParameter(trackNum, paramNum, offset); }
	else if (pageNum == 1) {
		unit.AdjustEffectParameter(trackNum, paramNum, offset); }
	else if (pageNum == 2) {
		unit.AdjustMixParameter(trackNum, paramNum, offset); }
	else {
		auto msg = fmt::sprintf("invalid parameter page %d", pageNum);
		throw std::runtime_error(msg); }}


}  // namespace cxl
}  // namespace rqdq
