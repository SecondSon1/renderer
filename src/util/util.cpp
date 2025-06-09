#include "util/util.hpp"

namespace renderer {

namespace util {

int Sign(double x) {
  constexpr double kEPS = 1e-6;
  if (x > kEPS) {
    return 1;
  }
  if (x < -kEPS) {
    return -1;
  }
  return 0;
}

bool AlmostEqual(double a, double b) {
  return Sign(a - b) == 0;
}

}  // namespace util

}  // namespace renderer
