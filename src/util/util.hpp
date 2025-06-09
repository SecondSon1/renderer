#pragma once

namespace renderer {

namespace util {

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

int Sign(double x);
bool AlmostEqual(double a, double b);

}  // namespace util

}  // namespace renderer
