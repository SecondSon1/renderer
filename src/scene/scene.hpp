#pragma once

#include <vector>
#include <utility>
#include "scene/mesh.hpp"
#include "scene/camera.hpp"
#include "scene/lighting.hpp"
#include "linalg.hpp"
#include "util/timer.hpp"

namespace renderer {

class Scene {
 public:
  Scene(std::vector<Mesh>&& meshes, Lighting&& lighting);
  virtual ~Scene() = default;

  virtual void Advance(util::Timer::SecondsUnit seconds_elapsed) = 0;

  const std::vector<Mesh>& GetMeshes() const;
  const Lighting& GetLighting() const;

 protected:
  std::vector<Mesh> meshes_;
  Lighting lighting_;
};

}  // namespace renderer
