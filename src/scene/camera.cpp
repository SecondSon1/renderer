#include "camera.hpp"

#include <cmath>
#include <numbers>
#include "linalg.hpp"

namespace renderer {

Mat4x4d Camera::GetProjMatrix(double aspect_ratio) const {
  double hor_fov = fov_degrees_ * std::numbers::pi / 180.0;
  double dist_between_camera_and_screen = 1 / std::tan(hor_fov / 2);
  double vert_fov = 2 * std::atan2(aspect_ratio, dist_between_camera_and_screen);

  constexpr double kProjectionPlaneZ = 1.0;
  double ndc_max_x = std::tan(hor_fov * 0.5) * kProjectionPlaneZ;
  double ndc_min_x = -ndc_max_x;
  double ndc_max_y = std::tan(vert_fov * 0.5) * kProjectionPlaneZ;
  double ndc_min_y = -ndc_max_y;
  double ndc_dx_inv = 1.0 / (ndc_max_x - ndc_min_x);
  double ndc_dy_inv = 1.0 / (ndc_max_y - ndc_min_y);

  double near_plane_twice = 2 * z_near_;
  double frustum_depth_inv = 1 / (z_far_ - z_near_);

  double x_scale = 2 * ndc_dx_inv;
  double y_scale = 2 * ndc_dy_inv;
  double x_offset_rel_z = (ndc_max_x + ndc_min_x) * ndc_dx_inv;
  double y_offset_rel_z = (ndc_max_y + ndc_min_y) * ndc_dy_inv;
  double z_scale = -(z_far_ + z_near_) * frustum_depth_inv;
  double z_offset_absolute = -near_plane_twice * z_far_ * frustum_depth_inv;
  double cds_divisor = -1;

  Mat4x4d proj_mat;
  proj_mat.row(0) << x_scale, 0.0, x_offset_rel_z, 0.0;
  proj_mat.row(1) << 0.0, y_scale, y_offset_rel_z, 0.0;
  proj_mat.row(2) << 0.0, 0.0, z_scale, z_offset_absolute;
  proj_mat.row(3) << 0.0, 0.0, cds_divisor, 0.0;

  return proj_mat;
}

}  // namespace renderer
