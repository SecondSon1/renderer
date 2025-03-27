#pragma once

#include <cstdint>
#include <vector>

namespace renderer {

enum Width : uint32_t;
enum Height : uint32_t;

struct Pixel {
  uint8_t r_;
  uint8_t g_;
  uint8_t b_;
};

struct PixelWithDepth : Pixel {
  float z_;

  operator Pixel() const;
};

namespace colors {

constexpr Pixel kWhite = {.r_ = 255, .g_ = 255, .b_ = 255};
constexpr Pixel kBlack = {.r_ = 0, .g_ = 0, .b_ = 0};

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
  std::vector<uint8_t> buf_;
  std::vector<float> z_buf_;
};

}  // namespace renderer
