#include "util/drawing_primitives.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <utility>
#include <glog/logging.h>
#include "util/util.hpp"

namespace renderer {

namespace util {

void DrawLine(ImageWithDepth& img, Index from, Index to, Pixel color) {
  assert(0 <= from.col_ && from.col_ < img.GetWidth());
  assert(0 <= to.col_ && to.col_ < img.GetWidth());
  assert(0 <= from.row_ && from.row_ < img.GetHeight());
  assert(0 <= to.row_ && to.row_ < img.GetHeight());
  int32_t x = from.col_;
  int32_t y = from.row_;
  int32_t x1 = to.col_;
  int32_t y1 = to.row_;
  int32_t x_dir = (x < x1 ? 1 : -1);
  int32_t y_dir = (y < y1 ? 1 : -1);
  int32_t dx = std::abs(x1 - x);
  int32_t dy = std::abs(y1 - y);
  int32_t error = dx - dy;
  img[Col(x), Row(y)] = color;

  while (!(x == x1 && y == y1)) {
    auto err2 = error * 2;
    if (err2 > -dy) {
      error -= dy;
      x += x_dir;
    }
    if (err2 < dx) {
      error += dx;
      y += y_dir;
    }
    img[Col(x), Row(y)] = color;
  }
}

namespace {

double InterpolateReciprocal(IndexWithDepth a, IndexWithDepth b, Index point) {
  int32_t px = static_cast<int32_t>(b.col_) - static_cast<int32_t>(a.col_);
  int32_t py = static_cast<int32_t>(b.row_) - static_cast<int32_t>(a.row_);
  int32_t cx = static_cast<int32_t>(point.col_) - static_cast<int32_t>(a.col_);
  int32_t cy = static_cast<int32_t>(point.row_) - static_cast<int32_t>(a.row_);
  /*
  double t_x = px == 0 ? 0 : (static_cast<double>(cx) / px);
  double t_y = py == 0 ? 0 : (static_cast<double>(cy) / py);
  double t = (t_x + t_y) / 2;
  */
  double t = (px * cx + py * cy) / (px * px + py * py);
  //double den = t / a.z_ + (1 - t) / b.z_;
  //return 1 / den;
  double res = a.z_ * b.z_ / (t * b.z_ + (1 - t) * a.z_);
  return res;
}

void FillScanline(ImageWithDepth& img, IndexWithDepth from, IndexWithDepth to, Pixel color) {
  assert(from.row_ == to.row_);
  assert(from.col_ <= to.col_);
  if (from.col_ == to.col_) {
    img[from].SetIfCloserToCamera(color, from.z_);
    return;
  }
  const double range_len_inv = 1 / (to.col_ - from.col_);
  for (size_t i = from.col_; i <= to.col_; ++i) {
    double t = (i - from.col_) * range_len_inv;
    assert(0 <= t && t <= 1);
    //double den = t / from.z_ + (1 - t) / to.z_;
    double depth = from.z_ * to.z_ / (t * to.z_ + (1 - t) * from.z_);
    img[from.row_, Col(i)].SetIfCloserToCamera(color, depth);
  }
}

void FillAreaBetweenTwoSegments(ImageWithDepth& img, IndexWithDepth point, IndexWithDepth v1,
                                IndexWithDepth v2, Pixel color) {
  int32_t x_left = point.col_;
  int32_t y_left = point.row_;
  int32_t x_right = x_left;
  int32_t y_right = y_left;
  const int32_t x_left_target = v1.col_;
  const int32_t y_left_target = v1.row_;
  const int32_t x_right_target = v2.col_;
  const int32_t y_right_target = v2.row_;

  const int32_t x_dir_left = (x_left < x_left_target ? 1 : -1);
  const int32_t y_dir_left = (y_left < y_left_target ? 1 : -1);
  const int32_t x_dir_right = (x_right < x_right_target ? 1 : -1);
  const int32_t y_dir_right = (y_right < y_right_target ? 1 : -1);

  const int32_t dx_left = std::abs(x_left - x_left_target);
  const int32_t dy_left = std::abs(y_left - y_left_target);
  const int32_t dx_right = std::abs(x_right - x_right_target);
  const int32_t dy_right = std::abs(y_right - y_right_target);
  int32_t error_left = dx_left - dy_left;
  int32_t error_right = dx_right - dy_right;
  img[Col(x_left), Row(y_left)].SetIfCloserToCamera(color, point.z_);

  while (y_left != y_left_target) {
    size_t current_y = y_left;
    assert(y_left == y_right);

    while (!(x_left == x_left_target && y_left == y_left_target)) {
      auto err2 = error_left * 2;
      if (err2 > -dy_left) {
        error_left -= dy_left;
        x_left += x_dir_left;
      }
      if (err2 < dx_left) {
        error_left += dx_left;
        y_left += y_dir_left;
        break;
      }
    }
    while (!(x_right == x_right_target && y_right == y_right_target)) {
      auto err2 = error_right * 2;
      if (err2 > -dy_right) {
        error_right -= dy_right;
        x_right += x_dir_right;
      }
      if (err2 < dx_right) {
        error_right += dx_right;
        y_right += y_dir_right;
        break;
      }
    }

    Index from_scanline_nodepth = (Col(x_left), Row(current_y));
    Index to_scanline_nodepth = (Col(x_right), Row(current_y));
    float from_depth = InterpolateReciprocal(point, v1, from_scanline_nodepth);
    float to_depth = InterpolateReciprocal(point, v2, to_scanline_nodepth);
    IndexWithDepth from_scanline = {from_scanline_nodepth, from_depth};
    IndexWithDepth to_scanline = {to_scanline_nodepth, to_depth};
    assert(std::min(point.z_, v1.z_) - 1e-3 <= from_depth &&
           from_depth <= std::max(point.z_, v1.z_) + 1e-3);
    assert(std::min(point.z_, v2.z_) - 1e-3 <= to_depth &&
           to_depth <= std::max(point.z_, v2.z_) + 1e-3);
    FillScanline(img, from_scanline, to_scanline, color);
  }

  FillScanline(img, v1, v2, color);
}

void FillTriangleFlatBottom(ImageWithDepth& img, IndexWithDepth top, IndexWithDepth bottom1,
                            IndexWithDepth bottom2, Pixel color) {
  // TODO: care about lighting
  assert(0 <= top.col_ && top.col_ < img.GetWidth());
  assert(0 <= bottom1.col_ && bottom1.col_ < img.GetWidth());
  assert(0 <= bottom2.col_ && bottom2.col_ < img.GetWidth());
  assert(0 <= top.row_ && top.row_ < img.GetHeight());
  assert(0 <= bottom1.row_ && bottom1.row_ < img.GetHeight());
  assert(0 <= bottom2.row_ && bottom2.row_ < img.GetHeight());

  assert(top.row_ >= bottom1.row_);
  assert(bottom1.row_ == bottom2.row_);
  if (bottom1.col_ > bottom2.col_) {
    std::swap(bottom1, bottom2);
  }
  FillAreaBetweenTwoSegments(img, top, bottom1, bottom2, color);
}
void FillTriangleFlatTop(ImageWithDepth& img, IndexWithDepth top1, IndexWithDepth top2,
                         IndexWithDepth bottom, Pixel color) {
  // TODO: care about lighting
  assert(0 <= top1.col_ && top1.col_ < img.GetWidth());
  assert(0 <= top2.col_ && top2.col_ < img.GetWidth());
  assert(0 <= bottom.col_ && bottom.col_ < img.GetWidth());
  assert(0 <= top1.row_ && top1.row_ < img.GetHeight());
  assert(0 <= top2.row_ && top2.row_ < img.GetHeight());
  assert(0 <= bottom.row_ && bottom.row_ < img.GetHeight());

  assert(top1.row_ == top2.row_);
  assert(bottom.row_ <= top1.row_);
  if (top1.col_ > top2.col_) {
    std::swap(top1, top2);
  }
  FillAreaBetweenTwoSegments(img, bottom, top1, top2, color);
}

}  // namespace

void FillTriangle(ImageWithDepth& img, IndexWithDepth v1, IndexWithDepth v2, IndexWithDepth v3,
                  Pixel color) {
  if (v2.row_ > v1.row_) {
    std::swap(v1, v2);
  }
  if (v3.row_ > v1.row_) {
    std::swap(v1, v3);
  }

  if (v1.row_ == v2.row_) {
    return FillTriangleFlatTop(img, v1, v2, v3, color);
  }
  if (v1.row_ == v3.row_) {
    return FillTriangleFlatTop(img, v3, v1, v2, color);
  }
  if (v2.row_ == v3.row_) {
    return FillTriangleFlatBottom(img, v1, v2, v3, color);
  }

  if (v3.row_ > v2.row_) {
    std::swap(v2, v3);
  }

  double d1 = v1.row_ - v2.row_;
  double d2 = v2.row_ - v3.row_;
  assert(d1 >= 0 && d2 >= 0);
  double sum = std::abs(static_cast<double>(v1.col_) - v3.col_);
  double t = d1 / (d1 + d2);
  assert(0 <= t && t <= 1);
  double x_from_v1 = sum * t;
  if (v3.col_ < v1.col_) {
    x_from_v1 *= -1;
  }
  Index v1_v3_intersect_nodepth = (Row(v2.row_), Col(std::round(v1.col_ + x_from_v1)));
  double depth_double = 1 / (t / v1.z_ + (1 - t) / v3.z_);
  float depth = static_cast<float>(depth_double);
  IndexWithDepth v1_v3_intersect = {v1_v3_intersect_nodepth, depth};
  FillTriangleFlatBottom(img, v1, v2, v1_v3_intersect, color);
  FillTriangleFlatTop(img, v1_v3_intersect, v2, v3, color);
  /*
  DrawLine(img, v1, v2, {255, 0, 0});
  DrawLine(img, v2, v3, {255, 0, 0});
  DrawLine(img, v1, v3, {255, 0, 0});
  */
}

}  // namespace util

}  // namespace renderer
