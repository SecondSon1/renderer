#pragma once

#include <cstdint>
#include <vector>

namespace renderer {

enum Width : uint32_t;
enum Height : uint32_t;

struct Pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

namespace colors {

constexpr Pixel kWhite = {.r = 255, .g = 255, .b = 255};
constexpr Pixel kBlack = {.r = 0, .g = 0, .b = 0};

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
class Image {
 private:
  class PixelReference;

 public:
  Image(Width width, Height height) noexcept;

  Width GetWidth() const;
  Height GetHeight() const;

  Pixel operator[](Index idx) const;
  PixelReference operator[](Index idx);

  const void *GetPixelBuffer() const;

 private:
  size_t GetPixelIndex(Index idx) const;

 private:
  class PixelReference {
   public:
    PixelReference(uint8_t *pixel) noexcept;

    operator Pixel() const noexcept;

    PixelReference operator=(Pixel pixel);

    PixelReference SetR(uint8_t r);
    PixelReference SetG(uint8_t g);
    PixelReference SetB(uint8_t b);

   private:
    uint8_t ReadR() const;
    uint8_t &ReadR();
    uint8_t ReadG() const;
    uint8_t &ReadG();
    uint8_t ReadB() const;
    uint8_t &ReadB();

   private:
    uint8_t *pixel_;
  };

  uint32_t width_;
  uint32_t height_;
  std::vector<uint8_t> buf_;
};

}  // namespace renderer
