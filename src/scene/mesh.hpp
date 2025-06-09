#pragma once

#include <vector>
#include <array>
#include <optional>
#include <Eigen/Core>
#include "fs/obj_parser.hpp"
#include "linalg.hpp"
#include "graphics/image.hpp"

namespace renderer {

struct Triangle : std::array<Vector3d, 3> {
  using Vector = Vector3d;
  using TexVector = Vector2d;
  using std::array<Vector, 3>::array;

  Triangle Transform(const Mat4x4d& mat) const;
  Vector3d GetNonUnitNormal() const;

  Pixel color_;
  std::optional<std::array<TexVector, 3>> texture_vertices_;
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
