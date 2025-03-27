#include <graphics/image.hpp>

#include <cassert>
#include <cstdint>
#include <limits>
#include "graphics/sdl_settings.hpp"

namespace renderer {

PixelWithDepth::operator Pixel() const {
  Pixel result;
  result.r_ = r_;
  result.g_ = g_;
  result.b_ = b_;
  return result;
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

ImageWithDepth::ImageWithDepth(Width width, Height height) noexcept
    : width_(width),
      height_(height),
      buf_(width * height * SDL::settings::pixel_fmt::kPixelSizeInBytes),
      z_buf_(width * height, std::numeric_limits<float>::infinity()) {
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
  return static_cast<const void*>(buf_.data());
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
  if (ReadZ() > pixel_depth) {
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
