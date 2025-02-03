#pragma once

#include <glog/logging.h>
#include <cassert>
#include <cstdlib>

#include <SDL3/SDL.h>

namespace SDL {
namespace Impl {
void EnsureSDLIsInitialized();
void QuitSDLIfNotNeeded();
}  // namespace Impl

struct Renderer;

struct Window {
  Window(const char* title, size_t width, size_t height, SDL_WindowFlags flags);
  ~Window();

  Renderer GetRenderer(const char* name);

  SDL_Window* window;
};

struct Renderer {
  Renderer(Window& window, const char* renderer_name);
  ~Renderer();

  SDL_Renderer* renderer;
};
}  // namespace SDL