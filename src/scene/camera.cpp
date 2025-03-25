#include "camera.hpp"
#include <linalg.hpp>

namespace renderer {

Mat4x4d Camera::GetProjMatrix(double aspect_ratio) const {
  double e = 1 / tan(fov / 2);

  double x_scale = aspect_ratio * e;
  double y_scale = e;
  double x_translation = 0;  // since camera is fixed to (0, 0, 0)
  double y_translation = 0;

  double z_range_inverse = 1 / (z_far - z_near);
  double z_scale = -(z_far + z_near) * z_range_inverse;
  double z_offset = -2 * z_near * z_far * z_range_inverse;
  Mat4x4d projMat;
  projMat << x_scale, 0, x_translation, 0, 0, y_scale, y_translation, 0, 0, 0, z_scale, z_offset, 0,
      0, -1, 0;
  return projMat;
}

}  // namespace renderer
