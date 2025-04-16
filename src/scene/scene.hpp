#pragma once

#include <vector>
#include "scene/mesh.hpp"
#include "scene/lighting.hpp"
#include "util/timer.hpp"
#include "fs/texture_reader.hpp"

namespace renderer {

class Scene {
 public:
  Scene(std::vector<Mesh>&& meshes, Lighting&& lighting, Texture&& tex);
  virtual ~Scene() = default;

  virtual void Advance(util::Timer::SecondsUnit seconds_elapsed) = 0;

  const std::vector<Mesh>& GetMeshes() const;
  const Lighting& GetLighting() const;
  const Texture& GetTexture() const;

 protected:
  std::vector<Mesh> meshes_;
  Lighting lighting_;
  Texture texture_;
};

}  // namespace renderer
