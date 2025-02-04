#include <scene/mesh.hpp>

constexpr double EPS = 1e-10;

// THIS IS INEFFICIENT
//   ... but for now we're only loading a single cube so it's fine
renderer::Mesh renderer::Mesh::Transform(const Eigen::Matrix4d& mat) const {
  Mesh result;
  result.triangles.reserve(triangles.size());
  for (auto& tri : triangles) {
    result.triangles.emplace_back();
    for (size_t i = 0; i < 3; ++i) {
      auto& orig = tri.pts[i];
      Eigen::Vector4d vec(orig[0], orig[1], orig[2], 1);
      Eigen::Vector4d vecTransformed = mat * vec;
      double w = vecTransformed[3];
      if (std::abs(w) > EPS) {
        vecTransformed *= 1 / w;
      }
      renderer::Triangle::Vector newVec(vecTransformed[0], vecTransformed[1], vecTransformed[2]);
      result.triangles.back().pts[i] = newVec;
    }
  }
  return result;
}
