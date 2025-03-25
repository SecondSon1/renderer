#include "scene/scene.hpp"

#include <cmath>
#include "linalg.hpp"

namespace renderer {

namespace {

Mat4x4d ExampleTransform(util::Timer::SecondsUnit elapsed) {
  Vector3d translate_by{};
  Mat4x4d rotationMatrixX = GetRotationMatrixX(elapsed * 2);
  Mat4x4d rotationMatrixZ = GetRotationMatrixZ(elapsed);

  return rotationMatrixX * rotationMatrixZ;
}

}  // namespace

void renderer::Scene::Advance(util::Timer::SecondsUnit seconds_elapsed) {
  // Example:
  time_since_start_ += seconds_elapsed;
  auto& mesh_to_transform = meshes_[0];
  auto example_transformation = ExampleTransform(seconds_elapsed);
  for (auto& tri : mesh_to_transform.triangles_) {
    tri = tri.Transform(example_transformation);
  }
}

}  // namespace renderer
