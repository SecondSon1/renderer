#include <scene/mesh.hpp>

#include <optional>
#include <variant>
#include <utility>
#include <vector>
#include <type_traits>
#include <glog/logging.h>

namespace renderer {

namespace {

bool IsAlmostZero(double x) {
  static constexpr double EPS = 1e-10;
  return std::abs(x) <= EPS;
}

}  // namespace

Triangle Triangle::Transform(const Mat4x4d& mat) const {
  Triangle result{};
  for (size_t i = 0; i < 3; ++i) {
    Eigen::Vector4d vec(0, 0, 0, 1);
    vec.head<3>() = (*this)[i];
    Eigen::Vector4d vecTransformed = mat * vec;
    auto w = vecTransformed[3];
    if (!IsAlmostZero(w)) {
      vecTransformed *= 1 / w;
    }
    Triangle::Vector newVec = vecTransformed.head<3>();
    result[i] = newVec;
  }
  return result;
}


Mesh Mesh::Transform(const Mat4x4d& mat) const {
  std::vector<Triangle> result;

  result.reserve(triangles_.size());
  for (auto& tri : triangles_) {
    result.emplace_back(tri.Transform(mat));
  }
  return {std::move(result)};
}

Triangle operator+(Triangle tri, Triangle::Vector offset) {
  tri[0] += offset;
  tri[1] += offset;
  tri[2] += offset;
  return tri;
}

Triangle operator+(Triangle::Vector offset, Triangle tri) {
  return tri + offset;
}

}  // namespace renderer
