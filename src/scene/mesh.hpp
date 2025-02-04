#pragma once

#include <Eigen/Core>

namespace renderer {

struct Triangle {
  using Vector = Eigen::Vector3f;
  Vector pts[3]{};
};

struct Mesh {
  std::vector<Triangle> triangles;

  Mesh Transform(const Eigen::Matrix4d& mat) const;
};

}  // namespace renderer
