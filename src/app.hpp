#pragma once

#include <memory>
#include "renderer.hpp"
#include "view.hpp"
#include "controller.hpp"
#include "scene/scene.hpp"
#include "scene/camera.hpp"
#include "graphics/image.hpp"
#include "graphics/sdl_wrap.hpp"
#include "util/timer.hpp"
#include "util/timings_measurer.hpp"

namespace renderer {

class Application {
 public:
  Application();

  void Run();

 private:
  std::unique_ptr<Scene> LoadScene() const;
  Camera InitializeCamera() const;

  void HandleEvent(SDL::Event&& event);
  void RenderImage();
  void DrawOptions();

  void Quit();

 private:
  Renderer renderer_;
  std::unique_ptr<Scene> scene_;
  Image last_image_;
  Camera camera_;
  Controller ctl_;
  View view_;
  util::Timer timer_;
  util::TimingsMeasurer frame_timings_measurer_;

  bool is_running_ = false;
  bool is_paused_ = false;
  bool is_wireframe_ = false;
};

}  // namespace renderer
