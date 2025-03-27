#include "scene/scene.hpp"

namespace renderer {

Scene::Scene(std::vector<Mesh>&& meshes, Lighting&& lighting)
    : meshes_(std::move(meshes)), lighting_(std::move(lighting)) {
}

const std::vector<Mesh>& Scene::GetMeshes() const {
  return meshes_;
}

const Lighting& Scene::GetLighting() const {
  return lighting_;
}

}  // namespace renderer
