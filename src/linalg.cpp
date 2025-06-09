#include "linalg.hpp"

#include <Eigen/Geometry>

namespace renderer {

Mat4x4d GetRotationMatrixX(double rad) {
  Eigen::Affine3d transform(Eigen::AngleAxisd(rad, Vector3d::UnitX()));
  return transform.matrix();
}

Mat4x4d GetRotationMatrixY(double rad) {
  Eigen::Affine3d transform(Eigen::AngleAxisd(rad, Vector3d::UnitY()));
  return transform.matrix();
}

Mat4x4d GetRotationMatrixZ(double rad) {
  Eigen::Affine3d transform(Eigen::AngleAxisd(rad, Vector3d::UnitZ()));
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
