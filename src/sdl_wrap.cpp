#include <sdl_wrap.hpp>

#include <atomic>

std::atomic<size_t> sdl_instances_count = 0;

void SDL::Impl::EnsureSDLIsInitialized() {
  if (sdl_instances_count.fetch_add(1)) {
    return;
  }
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    LOG(FATAL) << "Failed to initialize SDL: " << SDL_GetError();
  }
}
void SDL::Impl::QuitSDLIfNotNeeded() {
  if (sdl_instances_count.fetch_sub(1) == 1) {
    SDL_Quit();
  }
}

SDL::Window::Window(const char* title, size_t width, size_t height, SDL_WindowFlags flags) {
  Impl::EnsureSDLIsInitialized();
  window = SDL_CreateWindow(title, width, height, flags);
  if (!window) {
    LOG(FATAL) << "Failed to create window: " << SDL_GetError();
  }
}

SDL::Window::~Window() {
  assert(window);
  SDL_DestroyWindow(window);
  Impl::QuitSDLIfNotNeeded();
}
SDL::Renderer SDL::Window::GetRenderer(const char* name) {
  return {*this, name};
}

SDL::Renderer::Renderer(Window& window, const char* renderer_name) {
  Impl::EnsureSDLIsInitialized();
  renderer = SDL_CreateRenderer(window.window, renderer_name);
  if (!renderer) {
    LOG(FATAL) << "Failed to create window: " << SDL_GetError();
  }
}

SDL::Renderer::~Renderer() {
  assert(renderer);
  SDL_DestroyRenderer(renderer);
  Impl::QuitSDLIfNotNeeded();
}