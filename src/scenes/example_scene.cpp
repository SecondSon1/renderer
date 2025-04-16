#include "scenes/example_scene.hpp"

#include <cassert>
#include <cmath>
#include <utility>
#include <numbers>
#include "linalg.hpp"
#include "scene/lighting.hpp"

namespace renderer {

namespace example {

namespace {

std::vector<Mesh> InitialTransform(std::vector<Mesh>&& meshes) {
  std::vector<Mesh> transformed = std::move(meshes);
  transformed[0].local_zero_(2) = -8;
  transformed[1].local_zero_(2) = 8;
  transformed[2].local_zero_(2) = -8;
  transformed[3].local_zero_ << 50, -10, -15;

  Vector3d translation{};
  translation << 1, 1, 1;
  Mat4x4d transformation = GetTranslationMatrix(translation);
  transformed[0] = transformed[0].Transform(transformation);

  Mat4x4d teapot_transform = GetRotationMatrixX(std::numbers::pi / 2);
  transformed[1] = transformed[1].Transform(teapot_transform);

  Mat4x4d cottage_transform = GetRotationMatrixY(-std::numbers::pi / 3 * 1.7);
  transformed[3] = transformed[3].Transform(cottage_transform);

  return transformed;
}

Lighting GenerateLighting() {
  constexpr double kAmbientLightLevel = 0.3;
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
  Mat4x4d rotationMatrixY = GetRotationMatrixZ(elapsed);

  return rotationMatrixX * rotationMatrixY;
}

Mat4x4d ExampleTransform2(util::Timer::SecondsUnit elapsed) {
  Mat4x4d rotationMatrixZ = GetRotationMatrixZ(elapsed * 1.3);
  Mat4x4d rotationMatrixX = GetRotationMatrixX(elapsed);

  return rotationMatrixZ * rotationMatrixX;
}

Vector3d GetLightSource(double time_since_start) {
  constexpr double kLightingChangeSpeed = 0.2;
  double x = 0.3;
  double y = std::sin(time_since_start * kLightingChangeSpeed);
  double z = std::cos(time_since_start * kLightingChangeSpeed);
  return Vector3d(x, y, z);
}

}  // namespace

ExampleScene::ExampleScene(std::vector<Mesh>&& meshes, Texture&& texture)
    : Scene(InitialTransform(std::move(meshes)), GenerateLighting(), std::move(texture)),
      orig_meshes_(meshes_),
      orig_lighting_(lighting_) {
}

void ExampleScene::Advance(util::Timer::SecondsUnit seconds_elapsed) {
  time_since_start_ += seconds_elapsed;
  auto example_transformation = ExampleTransform(time_since_start_);
  auto example_transformation2 = ExampleTransform2(time_since_start_);

  meshes_[0] = orig_meshes_[0].Transform(example_transformation);
  meshes_[1] = orig_meshes_[1].Transform(example_transformation2);

  return;
  lighting_ = orig_lighting_;
  auto& changing_light_source = std::get<DirectionalLight>(lighting_[1]);
  changing_light_source.direction_ = GetLightSource(time_since_start_);
  changing_light_source.direction_.normalize();
}

}  // namespace example

}  // namespace renderer
