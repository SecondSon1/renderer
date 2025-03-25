#pragma once

#include <string>
#include "graphics/sdl_wrap.hpp"
#include "graphics/image.hpp"

namespace renderer {

class View {
 public:
  View(Width width, Height height, std::string title);

  void Display(const Image& image);

  const SDL::Window& GetWindow() const;
  const SDL::Renderer& GetRenderer() const;

 private:
  SDL::Window window_;
  SDL::Renderer renderer_;
};

}  // namespace renderer
