#include "scene/scene.hpp"

#include <utility>

namespace renderer {

Scene::Scene(std::vector<Mesh>&& meshes, Lighting&& lighting, Texture&& tex)
    : meshes_(std::move(meshes)), lighting_(std::move(lighting)), texture_(std::move(tex)) {
}

const std::vector<Mesh>& Scene::GetMeshes() const {
  return meshes_;
}

const Lighting& Scene::GetLighting() const {
  return lighting_;
}

const Texture& Scene::GetTexture() const {
  return texture_;
}

}  // namespace renderer
