#pragma once

#include <Eigen/Core>

namespace renderer {

struct Camera {
  Camera(double fov, double z_near, double z_far) : fov(fov), z_near(z_near), z_far(z_far) {
  }

  double fov;
  double z_near;
  double z_far;
};

}  // namespace renderer
