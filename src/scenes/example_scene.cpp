#include "scenes/example_scene.hpp"

#include <cassert>
#include <cmath>
#include <utility>
#include "linalg.hpp"
#include "scene/lighting.hpp"

namespace renderer {

namespace example {

namespace {

std::vector<Mesh> InitialTransform(std::vector<Mesh>&& meshes) {
  std::vector<Mesh> transformed = std::move(meshes);
  for (auto& obj : transformed) {
    bool big = obj.triangles_.size() > 300;
    obj.local_zero_(2) = big ? 8 : -8;
  }

  Vector3d translation{};
  translation << 2, 1, 1;
  Mat4x4d transformation = GetTranslationMatrix(translation);
  transformed[0] = transformed[0].Transform(transformation);

  return transformed;
}

Lighting GenerateLighting() {
  constexpr double kAmbientLightLevel = 0.1;
  constexpr double kMainLightLevel = 0.9;
  Lighting result{};
  result.emplace_back(AmbientLight{
      .intensity_ = kAmbientLightLevel,
  });
  result.emplace_back(DirectionalLight{
      .direction_ = Vector3d(1, 0, -1).normalized(),
      .intensity_ = kMainLightLevel,
  });
  return result;
}

Mat4x4d ExampleTransform(util::Timer::SecondsUnit elapsed) {
  Mat4x4d rotationMatrixX = GetRotationMatrixX(elapsed * 2);
  Mat4x4d rotationMatrixZ = GetRotationMatrixY(elapsed);

  return rotationMatrixX * rotationMatrixZ;
}

}  // namespace

ExampleScene::ExampleScene(std::vector<Mesh>&& meshes)
    : Scene(InitialTransform(std::move(meshes)), GenerateLighting()), orig_meshes_(meshes_) {
}

void ExampleScene::Advance(util::Timer::SecondsUnit seconds_elapsed) {
  time_since_start_ += seconds_elapsed;
  size_t mesh_to_transform = 0;
  auto example_transformation = ExampleTransform(time_since_start_);

  const auto& orig_mesh = orig_meshes_[mesh_to_transform].triangles_;
  auto& mesh_tt = meshes_[mesh_to_transform].triangles_;
  assert(orig_mesh.size() == mesh_tt.size());

  for (size_t idx = 0; idx < orig_mesh.size(); ++idx) {
    mesh_tt[idx] = orig_mesh[idx].Transform(example_transformation);
  }
}

}  // namespace example

}  // namespace renderer
