#pragma once

#include <istream>
#include <variant>
#include <optional>
#include <array>
#include "linalg.hpp"
#include "scene/mesh.hpp"

namespace renderer {

struct Mesh;

class ObjParser {
 public:
  struct VertexEntry : public renderer::Vector3d {
    using Vector3d::Vector3d;
  };
  struct VertexTextureEntry : public renderer::Vector2d {
    using Vector2d::Vector2d;
  };
  struct FaceEntry {
    using IndexArray = std::array<size_t, 4>;
    uint8_t sz_;
    IndexArray vertex_indices_;
    std::optional<IndexArray> texture_indices_;
    std::optional<IndexArray> normal_indices_;
  };
  struct UnsupportedEntry {};

  using Entry = std::variant<VertexEntry, VertexTextureEntry, FaceEntry, UnsupportedEntry>;

 public:
  ObjParser(std::istream& stream);

  std::optional<Entry> GetNext() noexcept;
  std::optional<Entry> GetNextSkipUnsupported() noexcept;

  Mesh ConstructMesh();

 private:
  std::istream* stream_;
};

}  // namespace renderer
