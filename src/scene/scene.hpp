#pragma once

#include <vector>
#include "scene/mesh.hpp"
#include "scene/camera.hpp"
#include "util/timer.hpp"

namespace renderer {

struct Scene {
  Scene(const std::vector<Mesh>& meshes) : meshes_(meshes) {
  }

  void Advance(util::Timer::SecondsUnit seconds_elapsed);

  std::vector<Mesh> meshes_;
  util::Timer::SecondsUnit time_since_start_ = 0;
};

}  // namespace renderer
