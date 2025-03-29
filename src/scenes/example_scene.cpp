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
      {
          .intensity_ = kAmbientLightLevel,
          .color_ = colors::kWhite,
      },
  });
  const Vector3d first_light_dir = Vector3d(0.2, -1, 0.4).normalized();
  Pixel first_light_color = Pixel::FromHDR(colors::kBlue * 0.3f);
  result.emplace_back(DirectionalLight{
      {
          .intensity_ = kMainLightLevel / 4,
          .color_ = first_light_color,
      },
      first_light_dir,
  });
  const Vector3d second_light_dir = Vector3d(1, 0, -1.5).normalized();
  Pixel second_light_color = {.r_ = 0xf1, .g_ = 0xeb, .b_ = 0xc8};
  result.emplace_back(DirectionalLight{
      {
          .intensity_ = kMainLightLevel / 4 * 3,
          .color_ = second_light_color,
      },
      second_light_dir,
  });
  return result;
}

Mat4x4d ExampleTransform(util::Timer::SecondsUnit elapsed) {
  Mat4x4d rotationMatrixX = GetRotationMatrixX(elapsed * 2);
  Mat4x4d rotationMatrixZ = GetRotationMatrixY(elapsed);

  return rotationMatrixX * rotationMatrixZ;
}

}  // namespace

ExampleScene::ExampleScene(std::vector<Mesh>&& meshes, Texture&& texture)
    : Scene(InitialTransform(std::move(meshes)), GenerateLighting(), std::move(texture)),
      orig_meshes_(meshes_),
      orig_lighting_(lighting_) {
}

void ExampleScene::Advance(util::Timer::SecondsUnit seconds_elapsed) {
  time_since_start_ += seconds_elapsed;
  size_t mesh_to_transform = 1;
  auto example_transformation = ExampleTransform(time_since_start_);

  const auto& orig_mesh = orig_meshes_[mesh_to_transform].triangles_;
  auto& mesh_tt = meshes_[mesh_to_transform].triangles_;
  assert(orig_mesh.size() == mesh_tt.size());

  for (size_t idx = 0; idx < orig_mesh.size(); ++idx) {
    mesh_tt[idx] = orig_mesh[idx].Transform(example_transformation);
  }

  return;
  constexpr double kLightingChangeSpeed = 0.2;
  lighting_ = orig_lighting_;
  auto& changing_light_source = std::get<DirectionalLight>(lighting_[1]);
  double x = 0.3;
  double y = std::sin(time_since_start_ * kLightingChangeSpeed);
  double z = std::cos(time_since_start_ * kLightingChangeSpeed);
  changing_light_source.direction_ = Vector3d(x, y, z);
  changing_light_source.direction_.normalize();
}

}  // namespace example

}  // namespace renderer
