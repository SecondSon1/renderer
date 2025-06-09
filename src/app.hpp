#pragma once

#include <memory>
#include <functional>
#include <vector>
#include "renderer.hpp"
#include "view.hpp"
#include "controller.hpp"
#include "scene/scene.hpp"
#include "scene/camera.hpp"
#include "graphics/image.hpp"
#include "graphics/sdl_wrap.hpp"
#include "util/timer.hpp"
#include "util/timings_measurer.hpp"
#include "fs/texture_reader.hpp"

namespace renderer {

using ScenePtr = std::unique_ptr<Scene>;
using SceneCreator = std::function<ScenePtr(std::vector<Mesh>&&, Texture&&)>;

class Application {
 public:
  Application(SceneCreator&& scene_creator);

  void Run();

 private:
  Camera InitializeCamera() const;
  std::unique_ptr<Scene> LoadScene(SceneCreator&& scene_creator) const;

  void HandleEvent(SDL::Event&& event);
  ImageWithDepth RenderImage() const;
  void DrawOptions();

  void Quit();

 private:
  Renderer renderer_;
  std::unique_ptr<Scene> scene_;
  ImageWithDepth last_image_;
  Camera camera_;
  Controller ctl_;
  View view_;
  util::Timer timer_;
  util::TimingsMeasurer frame_timings_measurer_;

  bool is_running_ = false;
  bool is_paused_ = true;
  bool is_wireframe_ = false;
};

}  // namespace renderer
