#pragma once

#include "linalg.hpp"

namespace renderer {

struct Camera {
  double fov;
  double z_near;
  double z_far;

  Mat4x4d GetProjMatrix(double aspect_ratio) const;
};

}  // namespace renderer
