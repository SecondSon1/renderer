#include <scene/mesh.hpp>

#include <optional>
#include <variant>
#include <utility>
#include <vector>
#include <type_traits>
#include <glog/logging.h>
#include "util/util.hpp"

namespace renderer {

Triangle Triangle::Transform(const Mat4x4d& mat) const {
  Triangle result{};
  result.color_ = color_;
  result.texture_vertices_ = texture_vertices_;
  for (size_t i = 0; i < 3; ++i) {
    Eigen::Vector4d vec(0, 0, 0, 1);
    vec.head<3>() = (*this)[i];
    Eigen::Vector4d vecTransformed = mat * vec;
    auto w = vecTransformed[3];
    if (util::Sign(w) != 0) {
      vecTransformed *= 1 / w;
    }
    Triangle::Vector newVec = vecTransformed.head<3>();
    result[i] = newVec;
  }
  return result;
}

Vector3d Triangle::GetNonUnitNormal() const {
  Vector3d side1 = (*this)[1] - (*this)[0];
  Vector3d side2 = (*this)[2] - (*this)[0];
  return side1.cross(side2);
}


Mesh Mesh::Transform(const Mat4x4d& mat) const {
  std::vector<Triangle> result;

  result.reserve(triangles_.size());
  for (auto& tri : triangles_) {
    result.emplace_back(tri.Transform(mat));
  }
  return {
    .triangles_ = std::move(result),
    .local_zero_ = local_zero_,
  };
}

Triangle operator+=(Triangle& tri, Triangle::Vector offset) {
  tri[0] += offset;
  tri[1] += offset;
  tri[2] += offset;
  return tri;
}

Triangle operator+(const Triangle& tri, Triangle::Vector offset) {
  Triangle res = tri;
  res += offset;
  return res;
}

Triangle operator+(Triangle::Vector offset, const Triangle& tri) {
  return tri + offset;
}

}  // namespace renderer
