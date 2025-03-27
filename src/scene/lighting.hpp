#pragma once

#include <variant>
#include "linalg.hpp"

namespace renderer {

struct AmbientLight {
  double intensity_;
};

struct DirectionalLight {
  Vector3d direction_;
  double intensity_;
};

struct PointLightSource {
  Vector3d position_;
  double intensity_;
};

using LightSource = std::variant<AmbientLight, DirectionalLight, PointLightSource>;
struct Lighting : std::vector<LightSource> {
  using std::vector<LightSource>::vector;
};

}  // namespace renderer
