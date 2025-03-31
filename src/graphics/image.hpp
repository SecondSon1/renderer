#pragma once

#include <cstdint>
#include <memory>
#include <algorithm>
#include <cassert>
#include <limits>
#include <cmath>
#include <glog/logging.h>
#include "graphics/sdl_settings.hpp"
#include "util/parallelism.hpp"
#include "util/util.hpp"

namespace renderer {

enum Width : uint32_t;
enum Height : uint32_t;

class HDRPixel;

struct Pixel {
  uint8_t r_;
  uint8_t g_;
  uint8_t b_;

  operator HDRPixel() const;

  static Pixel FromHDR(HDRPixel hdr_pixel);
};

class HDRPixel {
 public:
  HDRPixel() = default;
  HDRPixel(float r, float g, float b);

  explicit operator Pixel() const;

  HDRPixel &operator+=(HDRPixel rhs);
  HDRPixel &operator-=(HDRPixel rhs);
  HDRPixel &operator*=(float scalar);
  HDRPixel &operator*=(HDRPixel rhs);
  HDRPixel &operator/=(float scalar);

  void SetR(float new_r);
  void SetG(float new_g);
  void SetB(float new_b);

  static HDRPixel FromPixel(Pixel pixel);

 private:
  float r_ = 0;
  float g_ = 0;
  float b_ = 0;
};

HDRPixel operator+(HDRPixel lhs, HDRPixel rhs);
HDRPixel operator-(HDRPixel lhs, HDRPixel rhs);
HDRPixel operator*(HDRPixel pix, float scalar);
HDRPixel operator*(float scalar, HDRPixel pix);
HDRPixel operator/(HDRPixel pix, float scalar);

struct PixelWithDepth {
  uint8_t r_;
  uint8_t g_;
  uint8_t b_;
  float z_;

  operator Pixel() const;

  PixelWithDepth &operator=(Pixel pixel);
};

namespace colors {

constexpr Pixel kWhite = {.r_ = 255, .g_ = 255, .b_ = 255};
constexpr Pixel kBlack = {.r_ = 0, .g_ = 0, .b_ = 0};
constexpr Pixel kRed = {.r_ = 255, .g_ = 0, .b_ = 0};
constexpr Pixel kGreen = {.r_ = 0, .g_ = 255, .b_ = 0};
constexpr Pixel kBlue = {.r_ = 0, .g_ = 0, .b_ = 255};
constexpr Pixel kYellow = {.r_ = 255, .g_ = 255, .b_ = 0};
constexpr Pixel kCyan = {.r_ = 0, .g_ = 255, .b_ = 255};
constexpr Pixel kMagenta = {.r_ = 255, .g_ = 0, .b_ = 255};

}  // namespace colors

enum Row : uint32_t;
enum Col : uint32_t;

struct Index {
  Row row_;
  Col col_;
};

Index operator,(Row row, Col col) noexcept;
Index operator,(Col col, Row row) noexcept;

// Columns index from left to right, as usual
// However row index from bottom to top
class ImageWithDepth {
 private:
  class PixelReference;

 public:
  ImageWithDepth(Width width, Height height) noexcept;

  Width GetWidth() const;
  Height GetHeight() const;

  PixelWithDepth operator[](Index idx) const;
  PixelReference operator[](Index idx);

  const void *GetPixelBuffer() const;

 private:
  size_t GetPixelIndex(Index idx) const;

  struct Tag {};

 private:
  class PixelReference {
   public:
    PixelReference(uint8_t *pixel, float *z_entry, Tag) noexcept;

    operator PixelWithDepth() const noexcept;

    PixelReference operator=(Pixel pixel);
    PixelReference operator=(PixelWithDepth pixel);

    PixelReference SetR(uint8_t r);
    PixelReference SetG(uint8_t g);
    PixelReference SetB(uint8_t b);
    PixelReference SetZ(float z);

    PixelReference SetIfCloserToCamera(Pixel pixel, float pixel_depth);

   private:
    uint8_t ReadR() const;
    uint8_t &ReadR();
    uint8_t ReadG() const;
    uint8_t &ReadG();
    uint8_t ReadB() const;
    uint8_t &ReadB();
    float ReadZ() const;
    float &ReadZ();

   private:
    uint8_t *pixel_;
    float *z_;
  };

  uint32_t width_;
  uint32_t height_;
  std::unique_ptr<uint8_t[]> buf_;
  std::unique_ptr<float[]> z_buf_;
};

inline Pixel::operator HDRPixel() const {
  constexpr float uint8_range_inv = 1.f / 255.f;
  return {
      static_cast<float>(r_) * uint8_range_inv,
      static_cast<float>(g_) * uint8_range_inv,
      static_cast<float>(b_) * uint8_range_inv,
  };
}

inline Pixel Pixel::FromHDR(HDRPixel hdr_pixel) {
  return static_cast<Pixel>(hdr_pixel);
}

namespace {

inline float ClampTo01(float x) {
  return std::clamp(x, 0.f, 1.f);
}

}  // namespace

inline HDRPixel::HDRPixel(float r, float g, float b)
    : r_(ClampTo01(r)), g_(ClampTo01(g)), b_(ClampTo01(b)) {
}

inline HDRPixel::operator Pixel() const {
  constexpr float uint8_range = 255.f;
  return {
      .r_ = static_cast<uint8_t>(std::round(r_ * uint8_range)),
      .g_ = static_cast<uint8_t>(std::round(g_ * uint8_range)),
      .b_ = static_cast<uint8_t>(std::round(b_ * uint8_range)),
  };
}

inline HDRPixel& HDRPixel::operator+=(HDRPixel rhs) {
  SetR(r_ + rhs.r_);
  SetG(g_ + rhs.g_);
  SetB(b_ + rhs.b_);
  return *this;
}

inline HDRPixel& HDRPixel::operator-=(HDRPixel rhs) {
  SetR(r_ - rhs.r_);
  SetG(g_ - rhs.g_);
  SetB(b_ - rhs.b_);
  return *this;
}

inline HDRPixel& HDRPixel::operator*=(float scalar) {
  assert(util::Sign(scalar) >= 0);
  SetR(r_ * scalar);
  SetG(g_ * scalar);
  SetB(b_ * scalar);
  return *this;
}

inline HDRPixel& HDRPixel::operator*=(HDRPixel rhs) {
  SetR(r_ * rhs.r_);
  SetG(g_ * rhs.g_);
  SetB(b_ * rhs.b_);
  return *this;
}

inline HDRPixel& HDRPixel::operator/=(float scalar) {
  assert(util::Sign(scalar) > 0);
  return *this *= (1 / scalar);
}

inline void HDRPixel::SetR(float new_r) {
  r_ = ClampTo01(new_r);
}

inline void HDRPixel::SetG(float new_g) {
  g_ = ClampTo01(new_g);
}

inline void HDRPixel::SetB(float new_b) {
  b_ = ClampTo01(new_b);
}

inline HDRPixel HDRPixel::FromPixel(Pixel pixel) {
  return pixel;
}

inline PixelWithDepth::operator Pixel() const {
  Pixel result;
  result.r_ = r_;
  result.g_ = g_;
  result.b_ = b_;
  return result;
}

inline PixelWithDepth& PixelWithDepth::operator=(Pixel pixel) {
  r_ = pixel.r_;
  g_ = pixel.g_;
  b_ = pixel.b_;
  return *this;
}

inline HDRPixel operator+(HDRPixel lhs, HDRPixel rhs) {
  lhs += rhs;
  return lhs;
}

inline HDRPixel operator-(HDRPixel lhs, HDRPixel rhs) {
  lhs -= rhs;
  return lhs;
}

inline HDRPixel operator*(HDRPixel pix, float scalar) {
  pix *= scalar;
  return pix;
}

inline HDRPixel operator*(float scalar, HDRPixel pix) {
  pix *= scalar;
  return pix;
}

inline HDRPixel operator/(HDRPixel pix, float scalar) {
  pix /= scalar;
  return pix;
}

inline Index operator,(Row row, Col col) noexcept {
  return {
      .row_ = row,
      .col_ = col,
  };
}

inline Index operator,(Col col, Row row) noexcept {
  return (row, col);
}

namespace pixel_fmt = SDL::settings::pixel_fmt;

inline ImageWithDepth::ImageWithDepth(Width width, Height height) noexcept
    : width_(width),
      height_(height),
      buf_(util::FillParallel<uint8_t>(width * height * pixel_fmt::kPixelSizeInBytes, 0)),
      z_buf_(util::FillParallel<float>(width * height, std::numeric_limits<float>::infinity())) {
}

inline Width ImageWithDepth::GetWidth() const {
  return Width{width_};
}

inline Height ImageWithDepth::GetHeight() const {
  return Height{height_};
}

inline PixelWithDepth ImageWithDepth::operator[](Index idx) const {
  size_t pix_idx = GetPixelIndex(idx);
  const uint8_t* address = &buf_[pix_idx * SDL::settings::pixel_fmt::kPixelSizeInBytes];
  const float* z_address = &z_buf_[pix_idx];

  // const_cast is OK here since PixelReference here does not write to address, it only reads. No UB
  return PixelWithDepth(
      PixelReference(const_cast<uint8_t*>(address), const_cast<float*>(z_address), Tag{}));
}

inline ImageWithDepth::PixelReference ImageWithDepth::operator[](Index idx) {
  size_t pixel_idx = GetPixelIndex(idx);
  size_t data_idx = pixel_idx * SDL::settings::pixel_fmt::kPixelSizeInBytes;
  uint8_t* address = &buf_[data_idx];
  float* z_address = &z_buf_[pixel_idx];
  return {address, z_address, Tag{}};
}

inline const void* ImageWithDepth::GetPixelBuffer() const {
  return static_cast<const void*>(&buf_[0]);
}

inline size_t ImageWithDepth::GetPixelIndex(Index idx) const {
  assert(idx.col_ < width_);
  assert(idx.row_ < height_);
  size_t row_from_top = height_ - idx.row_ - 1;
  return row_from_top * width_ + idx.col_;
}

inline ImageWithDepth::PixelReference::PixelReference(uint8_t* pixel, float* z_entry,
                                               ImageWithDepth::Tag) noexcept
    : pixel_(pixel), z_(z_entry) {
}

inline ImageWithDepth::PixelReference::operator PixelWithDepth() const noexcept {
  PixelWithDepth result = {.z_ = ReadZ()};
  result.r_ = ReadR();
  result.g_ = ReadG();
  result.b_ = ReadB();
  return result;
}

inline ImageWithDepth::PixelReference ImageWithDepth::PixelReference::operator=(Pixel pixel) {
  ReadR() = pixel.r_;
  ReadG() = pixel.g_;
  ReadB() = pixel.b_;
  return *this;
}

inline ImageWithDepth::PixelReference ImageWithDepth::PixelReference::operator=(PixelWithDepth pixel) {
  ReadZ() = pixel.z_;
  return *this = Pixel(pixel);
}

inline ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetR(uint8_t r) {
  ReadR() = r;
  return *this;
}

inline ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetG(uint8_t g) {
  ReadG() = g;
  return *this;
}

inline ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetB(uint8_t b) {
  ReadB() = b;
  return *this;
}

inline ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetZ(float z) {
  ReadZ() = z;
  return *this;
}

inline ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetIfCloserToCamera(
    Pixel pixel, float pixel_depth) {
  float cur = ReadZ();
  if (cur > pixel_depth) {
    *this = pixel;
    ReadZ() = pixel_depth;
  }
  return *this;
}

inline uint8_t ImageWithDepth::PixelReference::ReadR() const {
  return *(pixel_ + SDL::settings::pixel_fmt::kROffsetInBytes);
}
inline uint8_t& ImageWithDepth::PixelReference::ReadR() {
  return *(pixel_ + SDL::settings::pixel_fmt::kROffsetInBytes);
}

inline uint8_t ImageWithDepth::PixelReference::ReadG() const {
  return *(pixel_ + SDL::settings::pixel_fmt::kGOffsetInBytes);
}
inline uint8_t& ImageWithDepth::PixelReference::ReadG() {
  return *(pixel_ + SDL::settings::pixel_fmt::kGOffsetInBytes);
}

inline uint8_t ImageWithDepth::PixelReference::ReadB() const {
  return *(pixel_ + SDL::settings::pixel_fmt::kBOffsetInBytes);
}
inline uint8_t& ImageWithDepth::PixelReference::ReadB() {
  return *(pixel_ + SDL::settings::pixel_fmt::kBOffsetInBytes);
}

inline float ImageWithDepth::PixelReference::ReadZ() const {
  return *z_;
}
inline float& ImageWithDepth::PixelReference::ReadZ() {
  return *z_;
}

}  // namespace renderer
