#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>
#include "fs/datalist_parser.hpp"
#include "scene/mesh.hpp"

namespace renderer {

class DataLoader {
 public:
  DataLoader(std::filesystem::path path_to_datalist);

  std::optional<Mesh> GetObject();
  std::vector<Mesh> GetAllObjects();

 private:
  std::ifstream OpenFile(std::filesystem::path path);

 private:
  std::ifstream datalist_file_;
  DatalistParser parser_;
};

}  // namespace renderer
