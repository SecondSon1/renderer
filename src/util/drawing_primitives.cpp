#include "util/drawing_primitives.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>
#include <glog/logging.h>
#include "util/util.hpp"

namespace renderer {

namespace util {

void DrawLine(ImageWithDepth& img, Index from, Index to, Pixel color) {
  assert(0 <= from.col_ && from.col_ < static_cast<uint32_t>(img.GetWidth()));
  assert(0 <= to.col_ && to.col_ < static_cast<uint32_t>(img.GetWidth()));
  assert(0 <= from.row_ && from.row_ < static_cast<uint32_t>(img.GetHeight()));
  assert(0 <= to.row_ && to.row_ < static_cast<uint32_t>(img.GetHeight()));
  if (from.row_ > to.row_ || (from.row_ == to.row_ && from.col_ > to.col_)) {
    std::swap(from, to);
  }
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

double FindTDiscrete(IndexWithDepth a, IndexWithDepth b, Index point) {
  int32_t px = static_cast<int32_t>(a.col_) - static_cast<int32_t>(b.col_);
  int32_t py = static_cast<int32_t>(a.row_) - static_cast<int32_t>(b.row_);
  if (px == 0 && py == 0) {
    return 0;
  }
  int32_t cx = static_cast<int32_t>(point.col_) - static_cast<int32_t>(b.col_);
  int32_t cy = static_cast<int32_t>(point.row_) - static_cast<int32_t>(b.row_);
  double t = static_cast<double>(px * cx + py * cy) / static_cast<double>(px * px + py * py);
  assert(0 <= t && t <= 1);
  /*
  t = std::min(1.0, t);
  t = std::max(0.0, t);
  LOG_EVERY_N(INFO, 1000) << a.col_ << " " << a.row_ << " " << b.col_ << " " << b.row_ << " "
                          << point.col_ << " " << point.row_ << ": d1=" << a.z_ << ", d2=" << b.z_
                          << "; t = " << t << ", result = " << res;
  */

  return 1 - t;
}

Pixel FetchColorFromTexture(const Texture& tex, Vector2d tex_cds) {
  assert(0 <= tex_cds[0] && tex_cds[0] <= 1);
  assert(0 <= tex_cds[1] && tex_cds[1] <= 1);
  uint32_t row = std::round(tex_cds[1] * (tex.GetHeight() - 1));
  uint32_t col = std::round(tex_cds[0] * (tex.GetWidth() - 1));
  return tex[Row(row), Col(col)];
}

void FillScanline(const Texture& tex, ImageWithDepth& img, IndexWithDepth from, IndexWithDepth to,
                  Pixel lighting_color, bool is_tex) {
  assert(from.row_ == to.row_);
  assert(from.col_ <= to.col_);
  if (from.col_ == to.col_) {
    auto color = HDRPixel::FromPixel(is_tex ? FetchColorFromTexture(tex, from.tex_) : colors::kWhite);
    color *= lighting_color;
    img[from.ToIndex()].SetIfCloserToCamera(Pixel::FromHDR(color), from.z_);
    return;
  }
  const double range_len_inv = 1.0 / static_cast<double>(to.col_ - from.col_);
  for (size_t i = from.col_; i <= to.col_; ++i) {
    double t = (i - from.col_) * range_len_inv;
    assert(0 <= t && t <= 1);
    float depth = static_cast<float>(t * to.z_ + (1 - t) * from.z_);
    // float depth = static_cast<float>(t * from.z_ + (1 - t) * to.z_);
    Vector2d tex_vec = (1 - t) * from.tex_ + t * to.tex_;
    auto color = HDRPixel::FromPixel(is_tex ? FetchColorFromTexture(tex, tex_vec) : colors::kWhite);
    color *= lighting_color;
    img[from.row_, Col(i)].SetIfCloserToCamera(Pixel::FromHDR(color), depth);
  }
}

std::vector<IndexWithDepth> line_buffer_l;
std::vector<IndexWithDepth> line_buffer_r;
void FillBufferWithLine(std::vector<IndexWithDepth>& buf, IndexWithDepth from, IndexWithDepth to) {
  if (from.row_ > to.row_ || (from.row_ == to.row_ && from.col_ > to.col_)) {
    std::swap(from, to);
  }
  buf.clear();
  int32_t x = from.col_;
  int32_t y = from.row_;
  int32_t x1 = to.col_;
  int32_t y1 = to.row_;
  int32_t x_dir = (x < x1 ? 1 : -1);
  int32_t y_dir = (y < y1 ? 1 : -1);
  int32_t dx = std::abs(x1 - x);
  int32_t dy = std::abs(y1 - y);
  int32_t error = dx - dy;

  while (!(x == x1 && y == y1)) {
    auto err2 = error * 2;
    int32_t prev_x = x;
    int32_t prev_y = y;
    if (err2 > -dy) {
      error -= dy;
      x += x_dir;
    }
    if (err2 < dx) {
      error += dx;
      y += y_dir;
    }
    if (y != prev_y) {
      Index pt_nodepth = (Col(prev_x), Row(prev_y));
      double t = FindTDiscrete(from, to, pt_nodepth);
      float pt_depth = static_cast<float>(t * to.z_ + (1 - t) * from.z_);
      IndexWithTex pt_tex = {pt_nodepth, (1 - t) * from.tex_ + t * to.tex_};
      IndexWithDepth res = {pt_tex, pt_depth};
      buf.emplace_back(std::move(res));
    }
  }
  buf.emplace_back(std::move(to));
}

void FillAreaBetweenTwoSegments(const Texture& tex, ImageWithDepth& img, IndexWithDepth point,
                                IndexWithDepth v1, IndexWithDepth v2, Pixel color, bool is_tex) {
  assert(v1.row_ == v2.row_);
  if (img.GetHeight() > line_buffer_l.capacity()) {
    line_buffer_l.reserve(
        std::max(2 * line_buffer_l.capacity(), static_cast<size_t>(img.GetHeight())));
    line_buffer_r.reserve(line_buffer_r.capacity());
  }

  FillBufferWithLine(line_buffer_l, point, v1);
  FillBufferWithLine(line_buffer_r, point, v2);
  assert(line_buffer_l.size() == line_buffer_r.size());
  size_t line_buffer_sz = line_buffer_l.size();
  for (size_t i = 0; i < line_buffer_sz; ++i) {
    FillScanline(tex, img, line_buffer_l[i], line_buffer_r[i], color, is_tex);
  }
}

void FillTriangleFlatBottom(const Texture& tex, ImageWithDepth& img, IndexWithDepth top,
                            IndexWithDepth bottom1, IndexWithDepth bottom2, Pixel color,
                            bool is_tex) {
  assert(0 <= top.col_ && top.col_ < static_cast<uint32_t>(img.GetWidth()));
  assert(0 <= bottom1.col_ && bottom1.col_ < static_cast<uint32_t>(img.GetWidth()));
  assert(0 <= bottom2.col_ && bottom2.col_ < static_cast<uint32_t>(img.GetWidth()));
  assert(0 <= top.row_ && top.row_ < static_cast<uint32_t>(img.GetHeight()));
  assert(0 <= bottom1.row_ && bottom1.row_ < static_cast<uint32_t>(img.GetHeight()));
  assert(0 <= bottom2.row_ && bottom2.row_ < static_cast<uint32_t>(img.GetHeight()));

  assert(top.row_ >= bottom1.row_);
  assert(bottom1.row_ == bottom2.row_);
  if (bottom1.col_ > bottom2.col_) {
    std::swap(bottom1, bottom2);
  }
  FillAreaBetweenTwoSegments(tex, img, top, bottom1, bottom2, color, is_tex);
  // DrawLine(img, bottom1, bottom2, {255, 0, 0});
}
void FillTriangleFlatTop(const Texture& tex, ImageWithDepth& img, IndexWithDepth top1,
                         IndexWithDepth top2, IndexWithDepth bottom, Pixel color, bool is_tex) {
  assert(0 <= top1.col_ && top1.col_ < static_cast<uint32_t>(img.GetWidth()));
  assert(0 <= top2.col_ && top2.col_ < static_cast<uint32_t>(img.GetWidth()));
  assert(0 <= bottom.col_ && bottom.col_ < static_cast<uint32_t>(img.GetWidth()));
  assert(0 <= top1.row_ && top1.row_ < static_cast<uint32_t>(img.GetHeight()));
  assert(0 <= top2.row_ && top2.row_ < static_cast<uint32_t>(img.GetHeight()));
  assert(0 <= bottom.row_ && bottom.row_ < static_cast<uint32_t>(img.GetHeight()));

  assert(top1.row_ == top2.row_);
  assert(bottom.row_ <= top1.row_);
  if (top1.col_ > top2.col_) {
    std::swap(top1, top2);
  }
  FillAreaBetweenTwoSegments(tex, img, bottom, top1, top2, color, is_tex);
  // DrawLine(img, top1, top2, {255, 0, 0});
}

}  // namespace

void FillTriangle(ImageWithDepth& img, IndexWithDepth v1, IndexWithDepth v2, IndexWithDepth v3,
                  const Texture& tex, Pixel lighting_color, bool is_tex) {
  if (v2.row_ > v1.row_) {
    std::swap(v1, v2);
  }
  if (v3.row_ > v1.row_) {
    std::swap(v1, v3);
  }

  if (v1.row_ == v2.row_) {
    return FillTriangleFlatTop(tex, img, v1, v2, v3, lighting_color, is_tex);
  }
  if (v1.row_ == v3.row_) {
    return FillTriangleFlatTop(tex, img, v3, v1, v2, lighting_color, is_tex);
  }
  if (v2.row_ == v3.row_) {
    return FillTriangleFlatBottom(tex, img, v1, v2, v3, lighting_color, is_tex);
  }

  if (v3.row_ > v2.row_) {
    std::swap(v2, v3);
  }

  double d1 = v1.row_ - v2.row_;
  double d2 = v2.row_ - v3.row_;
  assert(d1 >= 0 && d2 >= 0);
  double sum = std::abs(static_cast<double>(v1.col_) - static_cast<double>(v3.col_));
  double t = d1 / (d1 + d2);
  assert(0 <= t && t <= 1);
  double x_from_v1 = sum * t;
  if (v3.col_ < v1.col_) {
    x_from_v1 *= -1;
  }
  Index v1_v3_intersect_nodepth =
      (Row(v2.row_), Col(std::round(static_cast<int32_t>(v1.col_) + x_from_v1)));
  float depth = static_cast<float>(t * v3.z_ + (1 - t) * v1.z_);
  Vector2d tex_lerp = v1.tex_ * (1 - t) + v3.tex_ * t;
  IndexWithTex v1_v3_with_tex = {v1_v3_intersect_nodepth, tex_lerp};
  IndexWithDepth v1_v3_intersect = {v1_v3_with_tex, depth};
  FillTriangleFlatBottom(tex, img, v1, v2, v1_v3_intersect, lighting_color, is_tex);
  FillTriangleFlatTop(tex, img, v1_v3_intersect, v2, v3, lighting_color, is_tex);
  /*
  DrawLine(img, v1, v2, {255, 0, 0});
  DrawLine(img, v2, v3, {255, 0, 0});
  DrawLine(img, v1, v3, {255, 0, 0});
  */
}

}  // namespace util

}  // namespace renderer
