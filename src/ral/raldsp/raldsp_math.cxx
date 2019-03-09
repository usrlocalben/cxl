#include "src/ral/raldsp/raldsp_math.hxx"

#include <cmath>

namespace rqdq {
namespace {

/* 20 / ln(10) */
constexpr double kLogToDbFactor = 8.6858896380650365530225783783321;

/* ln(10) / 20 */
constexpr double kDbToLogFactor = 0.11512925464970228420089957273422;

}  // namespace

namespace raldsp {

double linear2db(double value) {
	return std::log(value) * kLogToDbFactor; }


double db2linear(double value) {
	return exp(value * kDbToLogFactor); }


}  // namespace raldsp
}  // namespace rqdq
