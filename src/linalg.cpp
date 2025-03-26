#include "linalg.hpp"

#include <Eigen/Geometry>

namespace renderer {

Mat4x4d renderer::GetRotationMatrixX(double rad) {
  Vector3d x_axis(1, 0, 0);
  Eigen::Affine3d transform(Eigen::AngleAxisd(rad, x_axis));
  return transform.matrix();
}

Mat4x4d renderer::GetRotationMatrixY(double rad) {
  Vector3d y_axis(0, 1, 0);
  Eigen::Affine3d transform(Eigen::AngleAxisd(rad, y_axis));
  return transform.matrix();
}

Mat4x4d renderer::GetRotationMatrixZ(double rad) {
  Vector3d z_axis(0, 0, 1);
  Eigen::Affine3d transform(Eigen::AngleAxisd(rad, z_axis));
  return transform.matrix();
}

Mat4x4d GetTranslationMatrix(Vector3d offset) {
  Mat4x4d result = Mat4x4d::Identity();
  result.col(3).head<3>() = offset;
  return result;
}

Mat4x4d GetScalingMatrix(Vector3d scaling) {
  Mat4x4d result = Mat4x4d::Identity();
  result(0, 0) = scaling(0);
  result(1, 1) = scaling(1);
  result(2, 2) = scaling(2);
  return result;
}

}  // namespace renderer
