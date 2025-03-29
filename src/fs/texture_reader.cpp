#include "fs/texture_reader.hpp"

#include <cassert>
#include <utility>
#include <glog/logging.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace renderer {

namespace {

struct STBDeleter {
  void operator()(unsigned char* buffer) const {
    stbi_image_free(buffer);
  }
};

}  // namespace

namespace impl {

struct Texture {
  Texture(Width width, Height height, std::unique_ptr<unsigned char, STBDeleter>&& ptr)
      : width_(width), height_(height), data_(std::move(ptr)) {
  }

  Width width_;
  Height height_;
  std::unique_ptr<unsigned char, STBDeleter> data_;
};

}  // namespace impl

Texture::Texture(std::unique_ptr<impl::Texture>&& impl) : impl_(std::move(impl)) {
}

Texture::Texture(Texture&& other) noexcept : impl_(std::exchange(other.impl_, nullptr)) {
}

Texture::~Texture() {
}

Pixel Texture::operator[](Index ind) const {
  size_t pix_ind = ind.row_ * GetWidth() + ind.col_;
  Pixel pixel;
  pixel.r_ = GetBuffer()[pix_ind * 3 + 0];
  pixel.g_ = GetBuffer()[pix_ind * 3 + 1];
  pixel.b_ = GetBuffer()[pix_ind * 3 + 2];
  return pixel;
}

Width Texture::GetWidth() const {
  return impl_->width_;
}

Height Texture::GetHeight() const {
  return impl_->height_;
}

const unsigned char* Texture::GetBuffer() const {
  return impl_->data_.get();
}

unsigned char* Texture::GetBuffer() {
  return impl_->data_.get();
}

TextureReader::~TextureReader() {
}

Texture TextureReader::LoadImage(std::filesystem::path path) {
  int channels;
  int width;
  int height;
  std::string path_str = path.string();
  unsigned char* buf = stbi_load(path_str.c_str(), &width, &height, &channels, 0);
  assert(channels == 3);  // r, g and b
  if (buf == NULL) {
    LOG(FATAL) << "Could not load texture from an image";
  }
  auto buf_wrapped = std::unique_ptr<unsigned char, STBDeleter>(buf, STBDeleter{});
  auto impl_tex =
      std::make_unique<impl::Texture>(Width(width), Height(height), std::move(buf_wrapped));
  return {std::move(impl_tex)};
}

}  // namespace renderer
