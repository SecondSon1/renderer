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
  Texture(std::unique_ptr<unsigned char, STBDeleter>&& ptr) : data_(std::move(ptr)) {
  }

  std::unique_ptr<unsigned char, STBDeleter> data_;
};

}  // namespace impl

Texture::Texture(Width width, Height height, std::unique_ptr<impl::Texture>&& impl)
    : impl_(std::move(impl)), width_(width), height_(height), buf_(impl_->data_.get()) {
}

Texture::Texture(Texture&& other) noexcept
    : impl_(std::exchange(other.impl_, nullptr)),
      width_(other.width_),
      height_(other.height_),
      buf_(std::exchange(other.buf_, nullptr)) {
}

Texture::~Texture() {
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
  auto impl_tex = std::make_unique<impl::Texture>(std::move(buf_wrapped));
  return {Width(width), Height(height), std::move(impl_tex)};
}

}  // namespace renderer
