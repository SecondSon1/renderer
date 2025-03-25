#pragma once

#include "renderer.hpp"
#include "view.hpp"
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
  Scene LoadScene() const;
  Camera InitializeCamera() const;

  void HandleEvent(SDL::Event&& event);
  void RenderImage();
  void DrawOptions();

 private:
  Renderer renderer_;
  Scene scene_;
  Image last_image_;
  Camera camera_;
  View view_;
  util::Timer timer_;
  util::TimingsMeasurer frame_timings_measurer_;

  bool is_running = false;
  bool is_paused = false;
  bool is_wireframe = false;
};

}  // namespace renderer
