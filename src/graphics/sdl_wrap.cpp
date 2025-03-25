#include "graphics/sdl_wrap.hpp"

#include <atomic>
#include <cassert>

#include <glog/logging.h>
#include "graphics/sdl_settings.hpp"
#include "graphics/image.hpp"

namespace SDL {

namespace Impl {

static std::atomic<size_t> sdl_instances_count = 0;

static void EnsureSDLIsInitialized() {
  if (sdl_instances_count.fetch_add(1))
    return;
  if (!SDL_Init(SDL_INIT_VIDEO))
    LOG(FATAL) << "Failed to initialize SDL: " << SDL_GetError();
}

static void QuitSDLIfNotNeeded() {
  if (sdl_instances_count.fetch_sub(1) == 1) {
    SDL_Quit();
  }
}

}  // namespace Impl

Window::Window(const char* title, size_t width, size_t height, SDL_WindowFlags flags)
    : width_(width), height_(height) {
  Impl::EnsureSDLIsInitialized();
  window_ = SDL_CreateWindow(title, width, height, flags);
  if (!window_) {
    LOG(FATAL) << "Failed to create window: " << SDL_GetError();
  }
}

Window::~Window() {
  assert(window_);
  SDL_DestroyWindow(window_);
  Impl::QuitSDLIfNotNeeded();
}
Renderer Window::GetRenderer(const char* name) {
  return {*this, name};
}

SDL_WindowID Window::GetID() const {
  auto result = SDL_GetWindowID(window_);
  if (!result) {
    LOG(FATAL) << "Could not obtain ID of the SDL window";
  }
  return result;
}

void Window::ShowWindow() {
  SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window_);
}

Renderer::Renderer(Window& window, const char* renderer_name) {
  Impl::EnsureSDLIsInitialized();
  renderer_ = SDL_CreateRenderer(window.window_, renderer_name);
  if (!renderer_) {
    LOG(FATAL) << "Failed to create window: " << SDL_GetError();
  }
  if (!SDL_SetRenderVSync(renderer_, 1)) {
    LOG(FATAL) << "Failed to enable vsync: " << SDL_GetError();
  }
  auto pixel_format = SDL_PIXELFORMAT_RGB24;
  assert(SDL::settings::pixel_fmt::kPixelSizeInBytes == 3);
  assert(SDL::settings::pixel_fmt::kROffsetInBytes == 0);
  assert(SDL::settings::pixel_fmt::kGOffsetInBytes == 1);
  assert(SDL::settings::pixel_fmt::kBOffsetInBytes == 2);

  main_tex_ = SDL_CreateTexture(renderer_, pixel_format, SDL_TEXTUREACCESS_STREAMING, window.width_,
                                window.height_);
  if (!main_tex_) {
    LOG(FATAL) << "Failed to create main texture: " << SDL_GetError();
  }
}

Renderer::~Renderer() {
  assert(main_tex_);
  assert(renderer_);
  SDL_DestroyTexture(main_tex_);
  SDL_DestroyRenderer(renderer_);
  Impl::QuitSDLIfNotNeeded();
}

void Renderer::Render(const renderer::Image& image) {
  const void* buffer = image.GetPixelBuffer();
  size_t pitch = image.GetWidth() * settings::pixel_fmt::kPixelSizeInBytes;
  bool result = SDL_UpdateTexture(main_tex_, NULL, buffer, pitch);
  if (!result) {
    LOG(FATAL) << "Could not update texture from image: " << SDL_GetError();
  }
  result = SDL_RenderTexture(renderer_, main_tex_, NULL, NULL);
  if (!result) {
    LOG(FATAL) << "Could not render texture: " << SDL_GetError();
  }
}

void Renderer::Present() {
  bool result = SDL_RenderPresent(renderer_);
  if (!result) {
    LOG(FATAL) << "Could not render present: " << SDL_GetError();
  }
}

}  // namespace SDL
