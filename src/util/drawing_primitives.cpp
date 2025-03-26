#include "util/drawing_primitives.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <utility>

namespace renderer {

namespace util {

void DrawLine(Image& img, Index from, Index to, Pixel color) {
  assert(0 <= from.col_ && from.col_ < img.GetWidth());
  assert(0 <= to.col_ && to.col_ < img.GetWidth());
  assert(0 <= from.row_ && from.row_ < img.GetHeight());
  assert(0 <= to.row_ && to.row_ < img.GetHeight());
  if (from.col_ > to.col_) {
    std::swap(from, to);
  }
  int32_t x = from.col_;
  int32_t y = from.row_;
  int32_t x1 = to.col_;
  int32_t y1 = to.row_;
  int32_t y_dir = (y < y1 ? 1 : -1);
  int32_t dx = std::abs(x1 - x);
  int32_t dy = std::abs(y1 - y);
  int32_t error = dx - dy;
  img[Col(x), Row(y)] = color;

  while (!(x == x1 && y == y1)) {
    auto err2 = error * 2;
    if (err2 > -dy) {
      error -= dy;
      ++x;
    }
    if (err2 < dx) {
      error += dx;
      y += y_dir;
    }
    img[Col(x), Row(y)] = color;
  }
}

}  // namespace util

}  // namespace renderer
