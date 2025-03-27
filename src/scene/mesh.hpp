#pragma once

#include <vector>
#include <array>
#include <Eigen/Core>
#include "fs/obj_parser.hpp"
#include "linalg.hpp"

namespace renderer {

struct Triangle :std::array<Vector3d, 3> {
  using Vector = Vector3d;
  using std::array<Vector, 3>::array;

  Triangle Transform(const Mat4x4d& mat) const;
};

Triangle operator+=(Triangle& tri, Triangle::Vector offset);
Triangle operator+(const Triangle& tri, Triangle::Vector offset);
Triangle operator+(Triangle::Vector offset, const Triangle& tri);

struct Mesh {
  Mesh Transform(const Mat4x4d& mat) const;

  std::vector<Triangle> triangles_;
  Vector3d local_zero_;
};

}  // namespace renderer
