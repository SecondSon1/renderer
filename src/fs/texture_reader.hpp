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
  Texture(std::unique_ptr<impl::Texture>&& impl);
  Texture(Texture&& other) noexcept;
  ~Texture();

  Pixel operator[](Index ind) const;

  Width GetWidth() const;
  Height GetHeight() const;
  const unsigned char* GetBuffer() const;
  unsigned char* GetBuffer();

 private:
  std::unique_ptr<impl::Texture> impl_;
};

class TextureReader {
 private:
  TextureReader() = delete;
  ~TextureReader();

 public:
  static Texture LoadImage(std::filesystem::path path);
};

}  // namespace renderer
