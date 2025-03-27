#pragma once

#include <memory>
#include <optional>
#include <SDL3/SDL.h>
#include "graphics/image.hpp"

namespace SDL {

struct Color {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
};

using Event = SDL_Event;
using KeyboardEvent = SDL_KeyboardEvent;
using MouseMotionEvent = SDL_MouseMotionEvent;
using MouseButtonEvent = SDL_MouseButtonEvent;
using MouseWheelEvent = SDL_MouseWheelEvent;

using Scancode = SDL_Scancode;

std::optional<Event> PollEvent();

class Renderer;

class Window {
  friend class Renderer;

 public:
  Window(const char* title, size_t width, size_t height, SDL_WindowFlags flags);
  ~Window();

  Renderer GetRenderer(const char* name);
  SDL_WindowID GetID() const;

  void ShowWindow();

 public:
  struct Flags {
    Flags() = delete;

    constexpr static SDL_WindowFlags kNone = 0;
    constexpr static SDL_WindowFlags kFullscreen = SDL_WINDOW_FULLSCREEN;
    constexpr static SDL_WindowFlags kHidden = SDL_WINDOW_HIDDEN;
    constexpr static SDL_WindowFlags kBorderless = SDL_WINDOW_BORDERLESS;
    constexpr static SDL_WindowFlags kResizable = SDL_WINDOW_RESIZABLE;
    constexpr static SDL_WindowFlags kInputFocus = SDL_WINDOW_INPUT_FOCUS;
    constexpr static SDL_WindowFlags kMouseFocus = SDL_WINDOW_MOUSE_FOCUS;
  };

 private:
  size_t width_;
  size_t height_;
  SDL_Window* window_;
};

class Renderer {
 public:
  Renderer(Window& window, const char* renderer_name);
  ~Renderer();

  void Render(const renderer::ImageWithDepth& image);
  void Present();

 private:
  SDL_Renderer* renderer_;
  SDL_Texture* main_tex_;
};

}  // namespace SDL
