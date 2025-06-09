#pragma once

#include <variant>
#include "linalg.hpp"
#include "graphics/image.hpp"

namespace renderer {

struct LightSourceBase {
  double intensity_;
  Pixel color_;
};

struct AmbientLight : public LightSourceBase {};

struct DirectionalLight : public LightSourceBase {
  Vector3d direction_;
};

struct PointLightSource : public LightSourceBase {
  Vector3d origin_;
};

struct LightSource : public std::variant<AmbientLight, DirectionalLight, PointLightSource> {
  using std::variant<AmbientLight, DirectionalLight, PointLightSource>::variant;

  LightSourceBase GetBase() const;
};

struct Lighting : std::vector<LightSource> {
  using std::vector<LightSource>::vector;
};

}  // namespace renderer
