#pragma once

#include <cstddef>
#include <memory>
#include <filesystem>
#include "graphics/image.hpp"

namespace renderer {

namespace impl {

struct Texture;

}

class Texture {
 public:
  Texture(Width width, Height height, std::unique_ptr<impl::Texture>&& impl);
  Texture(Texture&& other) noexcept;
  ~Texture();

  Pixel operator[](Index ind) const {
    assert(ind.row_ < GetHeight());
    assert(ind.col_ < GetWidth());
    size_t row_from_top = static_cast<size_t>(GetHeight()) - ind.row_ - 1;
    size_t pix_ind = row_from_top * GetWidth() + ind.col_;
    Pixel pixel;
    pixel.r_ = GetBuffer()[pix_ind * 3 + 0];
    pixel.g_ = GetBuffer()[pix_ind * 3 + 1];
    pixel.b_ = GetBuffer()[pix_ind * 3 + 2];
    return pixel;
  }

  Width GetWidth() const {
    return width_;
  }
  Height GetHeight() const {
    return height_;
  }
  const unsigned char* GetBuffer() const {
    return buf_;
  }
  unsigned char* GetBuffer() {
    return buf_;
  }

 private:
  std::unique_ptr<impl::Texture> impl_;
  Width width_;
  Height height_;
  unsigned char* buf_;
};

class TextureReader {
 private:
  TextureReader() = delete;
  ~TextureReader();

 public:
  static Texture LoadImage(std::filesystem::path path);
};

}  // namespace renderer
