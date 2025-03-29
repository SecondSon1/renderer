#pragma once

#include <vector>
#include "scene/scene.hpp"
#include "scene/lighting.hpp"
#include "scene/mesh.hpp"
#include "util/timer.hpp"

namespace renderer {

namespace example {

class ExampleScene : public Scene {
 public:
  ExampleScene(std::vector<Mesh>&& meshes, Texture&& texture);

  void Advance(util::Timer::SecondsUnit seconds_elapsed) override;

 private:
  util::Timer::SecondsUnit time_since_start_ = 0;
  std::vector<Mesh> orig_meshes_;
  Lighting orig_lighting_;
};

}  // namespace example

}  // namespace renderer
