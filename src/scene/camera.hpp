#pragma once

#include "linalg.hpp"

namespace renderer {

struct Camera {
  Mat4x4d GetProjMatrix(double aspect_ratio) const;
  double GetFOVInRadians() const;
  double ComputeDistToScreen() const;

  double fov_degrees_;
  double z_near_;
  double z_far_;
  bool inf_z_far_ = false;

  Vector3d pos_;
};

}  // namespace renderer
