#include <graphics/image.hpp>

#include <graphics/sdl_settings.hpp>
#include <cassert>
#include <cstdint>

namespace renderer {

Index operator,(Row row, Col col) noexcept {
  return {
      .row_ = row,
      .col_ = col,
  };
}

Index operator,(Col col, Row row) noexcept {
  return (row, col);
}

Image::Image(Width width, Height height) noexcept
    : width_(width),
      height_(height),
      buf_(width * height * SDL::settings::pixel_fmt::kPixelSizeInBytes) {
}

Width Image::GetWidth() const {
  return Width{width_};
}

Height Image::GetHeight() const {
  return Height{height_};
}

Pixel Image::operator[](Index idx) const {
  const uint8_t* address = &buf_[GetPixelIndex(idx) * SDL::settings::pixel_fmt::kPixelSizeInBytes];

  // const_cast is OK here since PixelReference here does not write to address, it only reads. No UB
  return Pixel(PixelReference(const_cast<uint8_t*>(address)));
}

Image::PixelReference Image::operator[](Index idx) {
  size_t pixel_idx = GetPixelIndex(idx);
  size_t data_idx = pixel_idx * SDL::settings::pixel_fmt::kPixelSizeInBytes;
  uint8_t* address = &buf_[data_idx];
  return {address};
}

const void* Image::GetPixelBuffer() const {
  return static_cast<const void*>(buf_.data());
}

size_t Image::GetPixelIndex(Index idx) const {
  assert(idx.col_ < width_);
  assert(idx.row_ < height_);
  size_t row_from_top = height_ - idx.row_ - 1;
  return row_from_top * width_ + idx.col_;
}

Image::PixelReference::PixelReference(uint8_t* pixel) noexcept : pixel_(pixel) {
}

Image::PixelReference::operator Pixel() const noexcept {
  return {.r = ReadR(), .g = ReadG(), .b = ReadB()};
}

Image::PixelReference Image::PixelReference::operator=(Pixel pixel) {
  ReadR() = pixel.r;
  ReadG() = pixel.g;
  ReadB() = pixel.b;
  return *this;
}

Image::PixelReference Image::PixelReference::SetR(uint8_t r) {
  ReadR() = r;
  return *this;
}

Image::PixelReference Image::PixelReference::SetG(uint8_t g) {
  ReadG() = g;
  return *this;
}

Image::PixelReference Image::PixelReference::SetB(uint8_t b) {
  ReadB() = b;
  return *this;
}

uint8_t Image::PixelReference::ReadR() const {
  return *(pixel_ + SDL::settings::pixel_fmt::kROffsetInBytes);
}
uint8_t& Image::PixelReference::ReadR() {
  return *(pixel_ + SDL::settings::pixel_fmt::kROffsetInBytes);
}

uint8_t Image::PixelReference::ReadG() const {
  return *(pixel_ + SDL::settings::pixel_fmt::kGOffsetInBytes);
}
uint8_t& Image::PixelReference::ReadG() {
  return *(pixel_ + SDL::settings::pixel_fmt::kGOffsetInBytes);
}

uint8_t Image::PixelReference::ReadB() const {
  return *(pixel_ + SDL::settings::pixel_fmt::kBOffsetInBytes);
}
uint8_t& Image::PixelReference::ReadB() {
  return *(pixel_ + SDL::settings::pixel_fmt::kBOffsetInBytes);
}

}  // namespace renderer
