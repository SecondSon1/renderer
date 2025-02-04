#include <scene/from_obj.hpp>

std::optional<renderer::Mesh> renderer::LoadMeshFromObjFile(const std::string& filepath) {
  std::ifstream input(filepath);
  if (!input.is_open()) {
    if (input.bad()) {
      LOG(ERROR) << "cannot read file " << filepath << ": bad bit is set";
    } else if (input.fail()) {
      LOG(ERROR) << "cannot read file " << filepath << ": " << strerror(errno);  // BRUH
    } else {
      LOG(ERROR) << "cannot read file " << filepath << " for unknown reasons :clown:";
    }

    return std::nullopt;
  }

  std::string file_line;

  std::vector<Eigen::Vector3f> vert;
  Mesh result{};

  auto ReadOneCoordOfFace = [](std::stringstream& line) -> size_t {
    char trash;
    int trash_int;
    size_t result;
    line >> result;
    assert(result > 0);  // 1-indexing
    if (line.peek() == '/') {
      line >> trash >> trash_int;
    }
    if (line.peek() == '/') {
      line >> trash >> trash_int;
    }
    return result - 1;
  };

  auto ReadOneFace =
      [&ReadOneCoordOfFace](std::stringstream& line) -> std::tuple<size_t, size_t, size_t> {
    return std::make_tuple(ReadOneCoordOfFace(line), ReadOneCoordOfFace(line),
                           ReadOneCoordOfFace(line));
  };

  while (std::getline(input, file_line)) {
    std::stringstream stream(file_line);
    std::string cmd;
    if (!(stream >> cmd)) {
      continue;
    }
    if (cmd.empty() || cmd[0] == '#') {
      continue;
    }

    if (cmd == "v") {  // vertex
      float x, y, z;
      stream >> x >> y >> z;
      vert.emplace_back(x, y, z);
    } else if (cmd == "f") {
      size_t v1, v2, v3;
      std::tie(v1, v2, v3) = ReadOneFace(stream);
      if (v1 >= vert.size() || v2 >= vert.size() || v3 >= vert.size()) {
        LOG(ERROR) << "triangle (face) is pointing at a non-existent vertex";
        return std::nullopt;
      }
      result.triangles.emplace_back(Triangle{
          .pts = {vert[v1], vert[v2], vert[v3]},
      });
    } else {
      LOG(WARNING) << "obj entry \"" << cmd << "\" not supported yet";
    }
  }

  return result;
}
