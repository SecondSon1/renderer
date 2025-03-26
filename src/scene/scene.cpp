#include "scene/scene.hpp"

namespace renderer {

Scene::Scene(std::vector<Mesh>&& meshes) : meshes_(std::move(meshes)) {
}

const std::vector<Mesh>& Scene::GetMeshes() const {
  return meshes_;
}

}  // namespace renderer
