#include <graphics/image.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <cmath>
#include <glog/logging.h>
#include "graphics/sdl_settings.hpp"
#include "util/parallelism.hpp"
#include "util/util.hpp"

namespace renderer {

Pixel::operator HDRPixel() const {
  constexpr float uint8_range_inv = 1.f / 255.f;
  return {
      static_cast<float>(r_) * uint8_range_inv,
      static_cast<float>(g_) * uint8_range_inv,
      static_cast<float>(b_) * uint8_range_inv,
  };
}

Pixel Pixel::FromHDR(HDRPixel hdr_pixel) {
  return static_cast<Pixel>(hdr_pixel);
}

namespace {

float ClampTo01(float x) {
  return std::clamp(x, 0.f, 1.f);
}

}  // namespace

HDRPixel::HDRPixel(float r, float g, float b)
    : r_(ClampTo01(r)), g_(ClampTo01(g)), b_(ClampTo01(b)) {
}

HDRPixel::operator Pixel() const {
  constexpr float uint8_range = 255.f;
  return {
      .r_ = static_cast<uint8_t>(std::round(r_ * uint8_range)),
      .g_ = static_cast<uint8_t>(std::round(g_ * uint8_range)),
      .b_ = static_cast<uint8_t>(std::round(b_ * uint8_range)),
  };
}

HDRPixel& HDRPixel::operator+=(HDRPixel rhs) {
  SetR(r_ + rhs.r_);
  SetG(g_ + rhs.g_);
  SetB(b_ + rhs.b_);
  return *this;
}

HDRPixel& HDRPixel::operator-=(HDRPixel rhs) {
  SetR(r_ - rhs.r_);
  SetG(g_ - rhs.g_);
  SetB(b_ - rhs.b_);
  return *this;
}

HDRPixel& HDRPixel::operator*=(float scalar) {
  assert(util::Sign(scalar) >= 0);
  SetR(r_ * scalar);
  SetG(g_ * scalar);
  SetB(b_ * scalar);
  return *this;
}

HDRPixel& HDRPixel::operator*=(HDRPixel rhs) {
  SetR(r_ * rhs.r_);
  SetG(g_ * rhs.g_);
  SetB(b_ * rhs.b_);
  return *this;
}

HDRPixel& HDRPixel::operator/=(float scalar) {
  assert(util::Sign(scalar) > 0);
  return *this *= (1 / scalar);
}

void HDRPixel::SetR(float new_r) {
  r_ = ClampTo01(new_r);
}

void HDRPixel::SetG(float new_g) {
  g_ = ClampTo01(new_g);
}

void HDRPixel::SetB(float new_b) {
  b_ = ClampTo01(new_b);
}

HDRPixel HDRPixel::FromPixel(Pixel pixel) {
  return pixel;
}

PixelWithDepth::operator Pixel() const {
  Pixel result;
  result.r_ = r_;
  result.g_ = g_;
  result.b_ = b_;
  return result;
}

PixelWithDepth& PixelWithDepth::operator=(Pixel pixel) {
  r_ = pixel.r_;
  g_ = pixel.g_;
  b_ = pixel.b_;
  return *this;
}

HDRPixel operator+(HDRPixel lhs, HDRPixel rhs) {
  lhs += rhs;
  return lhs;
}

HDRPixel operator-(HDRPixel lhs, HDRPixel rhs) {
  lhs -= rhs;
  return lhs;
}

HDRPixel operator*(HDRPixel pix, float scalar) {
  pix *= scalar;
  return pix;
}

HDRPixel operator*(float scalar, HDRPixel pix) {
  pix *= scalar;
  return pix;
}

HDRPixel operator/(HDRPixel pix, float scalar) {
  pix /= scalar;
  return pix;
}

Index operator,(Row row, Col col) noexcept {
  return {
      .row_ = row,
      .col_ = col,
  };
}

Index operator,(Col col, Row row) noexcept {
  return (row, col);
}

namespace pixel_fmt = SDL::settings::pixel_fmt;

ImageWithDepth::ImageWithDepth(Width width, Height height) noexcept
    : width_(width),
      height_(height),
      buf_(util::FillParallel<uint8_t>(width * height * pixel_fmt::kPixelSizeInBytes, 0)),
      z_buf_(util::FillParallel<float>(width * height, std::numeric_limits<float>::infinity())) {
}

Width ImageWithDepth::GetWidth() const {
  return Width{width_};
}

Height ImageWithDepth::GetHeight() const {
  return Height{height_};
}

PixelWithDepth ImageWithDepth::operator[](Index idx) const {
  size_t pix_idx = GetPixelIndex(idx);
  const uint8_t* address = &buf_[pix_idx * SDL::settings::pixel_fmt::kPixelSizeInBytes];
  const float* z_address = &z_buf_[pix_idx];

  // const_cast is OK here since PixelReference here does not write to address, it only reads. No UB
  return PixelWithDepth(
      PixelReference(const_cast<uint8_t*>(address), const_cast<float*>(z_address), Tag{}));
}

ImageWithDepth::PixelReference ImageWithDepth::operator[](Index idx) {
  size_t pixel_idx = GetPixelIndex(idx);
  size_t data_idx = pixel_idx * SDL::settings::pixel_fmt::kPixelSizeInBytes;
  uint8_t* address = &buf_[data_idx];
  float* z_address = &z_buf_[pixel_idx];
  return {address, z_address, Tag{}};
}

const void* ImageWithDepth::GetPixelBuffer() const {
  return static_cast<const void*>(&buf_[0]);
}

size_t ImageWithDepth::GetPixelIndex(Index idx) const {
  assert(idx.col_ < width_);
  assert(idx.row_ < height_);
  size_t row_from_top = height_ - idx.row_ - 1;
  return row_from_top * width_ + idx.col_;
}

ImageWithDepth::PixelReference::PixelReference(uint8_t* pixel, float* z_entry,
                                               ImageWithDepth::Tag) noexcept
    : pixel_(pixel), z_(z_entry) {
}

ImageWithDepth::PixelReference::operator PixelWithDepth() const noexcept {
  PixelWithDepth result = {.z_ = ReadZ()};
  result.r_ = ReadR();
  result.g_ = ReadG();
  result.b_ = ReadB();
  return result;
}

ImageWithDepth::PixelReference ImageWithDepth::PixelReference::operator=(Pixel pixel) {
  ReadR() = pixel.r_;
  ReadG() = pixel.g_;
  ReadB() = pixel.b_;
  return *this;
}

ImageWithDepth::PixelReference ImageWithDepth::PixelReference::operator=(PixelWithDepth pixel) {
  ReadZ() = pixel.z_;
  return *this = Pixel(pixel);
}

ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetR(uint8_t r) {
  ReadR() = r;
  return *this;
}

ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetG(uint8_t g) {
  ReadG() = g;
  return *this;
}

ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetB(uint8_t b) {
  ReadB() = b;
  return *this;
}

ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetZ(float z) {
  ReadZ() = z;
  return *this;
}

ImageWithDepth::PixelReference ImageWithDepth::PixelReference::SetIfCloserToCamera(
    Pixel pixel, float pixel_depth) {
  float cur = ReadZ();
  if (cur > pixel_depth) {
    *this = pixel;
    ReadZ() = pixel_depth;
  }
  return *this;
}

uint8_t ImageWithDepth::PixelReference::ReadR() const {
  return *(pixel_ + SDL::settings::pixel_fmt::kROffsetInBytes);
}
uint8_t& ImageWithDepth::PixelReference::ReadR() {
  return *(pixel_ + SDL::settings::pixel_fmt::kROffsetInBytes);
}

uint8_t ImageWithDepth::PixelReference::ReadG() const {
  return *(pixel_ + SDL::settings::pixel_fmt::kGOffsetInBytes);
}
uint8_t& ImageWithDepth::PixelReference::ReadG() {
  return *(pixel_ + SDL::settings::pixel_fmt::kGOffsetInBytes);
}

uint8_t ImageWithDepth::PixelReference::ReadB() const {
  return *(pixel_ + SDL::settings::pixel_fmt::kBOffsetInBytes);
}
uint8_t& ImageWithDepth::PixelReference::ReadB() {
  return *(pixel_ + SDL::settings::pixel_fmt::kBOffsetInBytes);
}

float ImageWithDepth::PixelReference::ReadZ() const {
  return *z_;
}
float& ImageWithDepth::PixelReference::ReadZ() {
  return *z_;
}

}  // namespace renderer
