#pragma once

#include "linalg.hpp"

namespace renderer {

struct Camera {
  Mat4x4d GetProjMatrix(double aspect_ratio) const;
  Mat4x4d GetWorldToCameraTransform() const;
  double GetFOVInRadians() const;
  double ComputeDistToScreen() const;
  Vector3d GetForwardDirection() const;
  Vector3d GetUpDirection() const;
  Vector3d GetRightDirection() const;

  void Offset(Vector3d shift);
  void RotateLookDir(Quaternion quat);

  double fov_degrees_;
  double z_near_;
  double z_far_;
  bool inf_z_far_ = false;

  Vector3d pos_;
  Quaternion dir_ = Quaternion::Identity();
};

}  // namespace renderer
