#pragma once

#include <scene/mesh.hpp>
#include <scene/camera.hpp>

namespace renderer {

struct Scene {
  Scene(const std::vector<Mesh>& meshes, const Camera& camera) : meshes_(meshes), camera_(camera) {
  }

  std::vector<Mesh> meshes_;
  Camera camera_;
};

}  // namespace renderer