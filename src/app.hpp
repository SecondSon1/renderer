#pragma once

#include "renderer.hpp"
#include "view.hpp"
#include "scene/scene.hpp"
#include "scene/camera.hpp"
#include "util/timer.hpp"

namespace renderer {

class Application {
 public:
  Application();

  void Run();

 private:
  Scene LoadScene() const;
  Camera InitializeCamera() const;

 private:
  Renderer renderer_;
  Scene scene_;
  Camera camera_;
  View view_;
  util::Timer timer_;
};

}  // namespace renderer
