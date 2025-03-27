#include "camera.hpp"

#include <cmath>
#include <numbers>
#include "linalg.hpp"

namespace renderer {

Mat4x4d Camera::GetProjMatrix(double aspect_ratio) const {
  double hor_fov = GetFOVInRadians();
  double dist_between_camera_and_screen = 1 / std::tan(hor_fov / 2);
  double vert_fov = 2 * std::atan2(aspect_ratio, dist_between_camera_and_screen);

  constexpr double kProjectionPlaneZ = 1.0;
  double ndc_max_x = std::tan(hor_fov * 0.5) * kProjectionPlaneZ;
  double ndc_min_x = -ndc_max_x;
  double ndc_max_y = std::tan(vert_fov * 0.5) * kProjectionPlaneZ;
  double ndc_min_y = -ndc_max_y;
  double ndc_dx_inv = 1.0 / (ndc_max_x - ndc_min_x);
  double ndc_dy_inv = 1.0 / (ndc_max_y - ndc_min_y);

  double frustum_depth_inv = 1 / (z_far_ - z_near_);

  double x_scale = 2 * ndc_dx_inv;
  double y_scale = 2 * ndc_dy_inv;
  double x_offset_rel_z = (ndc_max_x + ndc_min_x) * ndc_dx_inv;
  double y_offset_rel_z = (ndc_max_y + ndc_min_y) * ndc_dy_inv;

  double z_scale = -1;
  double z_offset_absolute = -2 * z_near_;
  if (!inf_z_far_) {
    z_scale *= (z_far_ + z_near_) * frustum_depth_inv;
    z_offset_absolute *= z_far_ * frustum_depth_inv;
  }

  double cds_divisor = -1;

  Mat4x4d proj_mat;
  proj_mat.row(0) << x_scale, 0.0, x_offset_rel_z, 0.0;
  proj_mat.row(1) << 0.0, y_scale, y_offset_rel_z, 0.0;
  proj_mat.row(2) << 0.0, 0.0, z_scale, z_offset_absolute;
  proj_mat.row(3) << 0.0, 0.0, cds_divisor, 0.0;

  return proj_mat;
}

Mat4x4d Camera::GetWorldToCameraTransform() const {
  Mat4x4d translation = Mat4x4d::Identity();
  translation.col(3).head<3>() = -pos_;
  Mat3x3d rotation = dir_.normalized().toRotationMatrix().transpose();
  Mat4x4d rotation_affine = Mat4x4d::Identity();
  rotation_affine.row(0).head<3>() = rotation.row(0);
  rotation_affine.row(1).head<3>() = rotation.row(1);
  rotation_affine.row(2).head<3>() = rotation.row(2);
  return rotation_affine * translation;
}

double Camera::GetFOVInRadians() const {
  return fov_degrees_ * (std::numbers::pi / 180.0);
}

double Camera::ComputeDistToScreen() const {
  return 1 / std::tan(GetFOVInRadians() / 2);
}

Vector3d Camera::GetForwardDirection() const {
  return -dir_.normalized().toRotationMatrix().col(2);
}

Vector3d Camera::GetUpDirection() const {
  return dir_.normalized().toRotationMatrix().col(1);
}

Vector3d Camera::GetRightDirection() const {
  return dir_.normalized().toRotationMatrix().col(0);
}

void Camera::Offset(Vector3d shift) {
  pos_ += shift;
}

void Camera::RotateLookDir(Quaternion quat) {
  dir_ *= quat.normalized();
}

}  // namespace renderer
