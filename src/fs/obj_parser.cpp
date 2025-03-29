#include "fs/obj_parser.hpp"

#include <utility>
#include <sstream>
#include <optional>
#include <glog/logging.h>
#include "util/util.hpp"

namespace renderer {

namespace {

ObjParser::VertexEntry GetVertexEntry(std::istream& in_stream) {
  ObjParser::VertexEntry result{};
  if (!(in_stream >> result[0] >> result[1] >> result[2])) {
    LOG(FATAL) << "wrong format for vertex in an .obj file: should be three floating-point values";
  }
  return result;
}

ObjParser::VertexTextureEntry GetVertexTextureEntry(std::istream& in_stream) {
  ObjParser::VertexTextureEntry result{};
  if (!(in_stream >> result[0] >> result[1])) {
    LOG(FATAL)
        << "wrong format for vertex texture in an .obj file: should be two floating-point values";
  }
  return result;
}

void ReadOnePartOfFaceEntry(std::istream& in_stream, size_t& vertex_index,
                            std::optional<size_t>& texture_index,
                            std::optional<size_t>& normal_index) {
  if (!(in_stream >> vertex_index)) {
    LOG(FATAL)
        << "wrong format for face in an .obj file: should specify all 3 indices for vertices";
  }

  int split = in_stream.get();
  if (split != '/') {
    return;
  }
  if (in_stream.peek() != '/') {
    size_t tex_ind;
    if (!(in_stream >> tex_ind)) {
      LOG(FATAL) << "wrong format for face in an .obj file: after '/' should follow texture "
                    "coordinate or another '/'";
    }
    texture_index = tex_ind;
  }
  if ((split = in_stream.get()) != '/') {
    return;
  }
  if (split == '/') {
    size_t norm_ind;
    if (!(in_stream >> norm_ind)) {
      LOG(FATAL)
          << "wrong format for face in an .obj file: after '/' should follow normal coordinate";
    }
    normal_index = norm_ind;
  }
}

ObjParser::FaceEntry GetFaceEntry(std::istream& in_stream) {
  using IndexArray = ObjParser::FaceEntry::IndexArray;
  ObjParser::FaceEntry result{.vertex_indices_ = IndexArray{},
                              .texture_indices_ = IndexArray{},
                              .normal_indices_ = IndexArray{}};

  auto& vertex_indices = result.vertex_indices_;
  auto& texture_indices = result.texture_indices_.value();
  auto& normal_indices = result.normal_indices_.value();

  size_t texture_inds_count = 0;
  size_t normal_inds_count = 0;

  for (size_t i = 0; i < 3; ++i) {
    std::optional<size_t> texture_proxy;
    std::optional<size_t> normal_proxy;
    ReadOnePartOfFaceEntry(in_stream, vertex_indices[i], texture_proxy, normal_proxy);
    if (texture_proxy) {
      ++texture_inds_count;
      texture_indices[i] = texture_proxy.value();
    }
    if (normal_proxy) {
      ++normal_inds_count;
      normal_indices[i] = normal_proxy.value();
    }
  }

  if (texture_inds_count == 0) {
    result.texture_indices_ = std::nullopt;
  } else if (texture_inds_count < 3) {
    LOG(FATAL) << "wrong format for face in an .obj file: can only supply either none or all 3 "
                  "texture indices";
  }
  if (normal_inds_count == 0) {
    result.normal_indices_ = std::nullopt;
  } else if (normal_inds_count < 3) {
    LOG(FATAL) << "wrong format for face in an .obj file: can only supply either none or all 3 "
                  "normal indices";
  }

  return result;
}

}  // namespace

ObjParser::ObjParser(std::istream& stream) : stream_(&stream) {
}

std::optional<ObjParser::Entry> ObjParser::GetNext() noexcept {
  std::string line;
  if (!std::getline(*stream_, line)) {
    return std::nullopt;
  }

  std::istringstream in(line);
  std::string cmd;
  if (!(in >> cmd)) {
    return GetNext();
  }
  if (cmd.empty() || cmd[0] == '#') {
    return GetNext();
  }

  if (cmd == "v") {
    return GetVertexEntry(in);
  } else if (cmd == "f") {
    return GetFaceEntry(in);
  } else if (cmd == "vt") {
    return GetVertexTextureEntry(in);
  } else {
    return UnsupportedEntry{};
  }
}

std::optional<ObjParser::Entry> ObjParser::GetNextSkipUnsupported() noexcept {
  auto result = GetNext();
  while (result && std::holds_alternative<ObjParser::UnsupportedEntry>(result.value())) {
    result = GetNext();
  }
  return result;
}

namespace {

template <typename T, typename... U>
using are_all_same = std::integral_constant<bool, (... && std::is_same_v<T, U>)>;

template <typename... Args>
  requires(are_all_same<size_t, Args...>::value)
bool AreIndicesCorrect(size_t current_count, Args... inds) {
  return (... && (inds < current_count));
}

Triangle::Vector GetVertexFromEntry(ObjParser::VertexEntry entry) {
  return {entry};
}

Triangle::TexVector GetVertexTextureFromEntry(ObjParser::VertexTextureEntry entry) {
  return {entry};
}

Triangle GetFaceFromEntry(ObjParser::FaceEntry entry, const std::vector<Triangle::Vector>& vertices,
                          const std::vector<Triangle::TexVector>& tex_vertices) {
  const auto& vert_indices = entry.vertex_indices_;
  size_t vert1 = vert_indices[0];
  size_t vert2 = vert_indices[1];
  size_t vert3 = vert_indices[2];
  if (vert1 == 0 || vert2 == 0 || vert3 == 0) {
    LOG(FATAL) << "wrong format for face in an .obj file: indexing is 1-based for indices "
                  "referenced by the face";
  }
  --vert1;
  --vert2;
  --vert3;

  if (!AreIndicesCorrect(vertices.size(), vert1, vert2, vert3)) {
    LOG(FATAL) << "wrong format for face in an .obj file: current index is out of bounds. Face can "
                  "only index vertices that came earlier in the file";
  }

  Triangle result;
  result[0] = vertices[vert1];
  result[1] = vertices[vert2];
  result[2] = vertices[vert3];

  const auto& tex_indices_maybe = entry.texture_indices_;
  if (tex_indices_maybe) {
    const auto& tex_indices = tex_indices_maybe.value();
    size_t tex_vert1 = tex_indices[0];
    size_t tex_vert2 = tex_indices[1];
    size_t tex_vert3 = tex_indices[2];

    if (tex_vert1 == 0 || tex_vert2 == 0 || tex_vert3 == 0) {
      LOG(FATAL)
          << "wrong format for face in an .obj file: indexing is 1-based for texture indices "
             "referenced by the face";
    }
    --tex_vert1;
    --tex_vert2;
    --tex_vert3;

    if (!AreIndicesCorrect(tex_vertices.size(), tex_vert1, tex_vert2, tex_vert3)) {
      LOG(FATAL)
          << "wrong format for face in an .obj file: current index is out of bounds. Face can "
             "only index texture vertices that came earlier in the file";
    }

    std::array<Triangle::TexVector, 3> tex_result;
    tex_result[0] = tex_vertices[tex_vert1];
    tex_result[1] = tex_vertices[tex_vert2];
    tex_result[2] = tex_vertices[tex_vert3];
    result.texture_vertices_ = std::move(tex_result);
  }
  return result;
}

}  // namespace

Mesh ObjParser::ConstructMesh() {
  std::optional<ObjParser::Entry> next_entry;

  std::vector<Triangle::Vector> vertices;
  std::vector<Triangle::TexVector> texture_vertices;
  std::vector<Triangle> result;

  auto handlers =
      util::overloaded{[&vertices](ObjParser::VertexEntry entry) {
                         vertices.emplace_back(GetVertexFromEntry(entry));
                       },
                       [&texture_vertices](ObjParser::VertexTextureEntry entry) {
                         texture_vertices.emplace_back(GetVertexTextureFromEntry(entry));
                       },
                       [&vertices, &result, &texture_vertices](ObjParser::FaceEntry entry) {
                         result.emplace_back(GetFaceFromEntry(entry, vertices, texture_vertices));
                       },
                       [](ObjParser::UnsupportedEntry) {}};

  while ((next_entry = GetNextSkipUnsupported())) {
    std::visit(handlers, next_entry.value());
    /* === Following code is equivalent to std::visit call ===
    auto entry = std::move(next_entry.value());
    if (std::holds_alternative<ObjParser::VertexEntry>(entry)) {
      auto vertex_entry = std::get<ObjParser::VertexEntry>(entry);
      ;
    } else if (std::holds_alternative<ObjParser::FaceEntry>(entry)) {
      auto face_entry = std::get<ObjParser::FaceEntry>(entry);
      result.emplace_back(GetFaceFromEntry(face_entry, vertices));
    } else if (!std::holds_alternative<ObjParser::UnsupportedEntry>(entry)) {
      LOG(FATAL) << "Forgot to handle another case in Mesh::ConstructFromFile";
    }
    */
  }
  return {std::move(result)};
}

}  // namespace renderer
