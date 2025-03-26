#pragma once

#include <Eigen/Core>

namespace renderer {

using Vector2f = Eigen::Vector2f;
using Vector2d = Eigen::Vector2d;
using Vector3f = Eigen::Vector3f;
using Vector3d = Eigen::Vector3d;
using Vector4f = Eigen::Vector4f;
using Vector4d = Eigen::Vector4d;

using Mat4x4f = Eigen::Matrix4f;
using Mat4x4d = Eigen::Matrix4d;

Mat4x4d GetRotationMatrixX(double rad);
Mat4x4d GetRotationMatrixY(double rad);
Mat4x4d GetRotationMatrixZ(double rad);
Mat4x4d GetTranslationMatrix(Vector3d offset);
Mat4x4d GetScalingMatrix(Vector3d scaling);

}  // namespace renderer
