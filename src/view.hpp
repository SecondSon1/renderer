#pragma once

#include <string>
#include "controller.hpp"
#include "graphics/sdl_wrap.hpp"
#include "graphics/image.hpp"
#include "scene/camera.hpp"
#include "util/timer.hpp"

namespace renderer {

class View {
 public:
  View(Width width, Height height, std::string title);

  void Display(const ImageWithDepth& image);
  void BeginFrame();

  void DrawWireframeOption(bool& wireframe);
  void DrawControllerOptions(Controller& ctl);
  void DrawCameraOptions(Camera& camera);
  void DrawFPS(util::Timer::SecondsUnit avg_frame_time);
  void DrawPauseOption(bool& paused);

  const SDL::Window& GetWindow() const;
  const SDL::Renderer& GetRenderer() const;

 private:
  void StartOptionsWindow();
  void EndOptionsWindow() const;

 private:
  SDL::Window window_;
  SDL::Renderer renderer_;
  bool options_drawn_;
};

}  // namespace renderer
