#include "fs/loader.hpp"

#include <glog/logging.h>
#include "fs/obj_parser.hpp"

namespace renderer {

DataLoader::DataLoader(std::filesystem::path path_to_datalist)
    : datalist_file_(OpenFile(path_to_datalist)), parser_(datalist_file_) {
}

std::ifstream DataLoader::OpenFile(std::filesystem::path path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    if (file.bad()) {
      LOG(FATAL) << "cannot read file " << path << ": bad bit is set";
    } else if (file.fail()) {
      LOG(FATAL) << "cannot read file " << path << ": " << strerror(errno);  // BRUH
    } else {
      LOG(FATAL) << "cannot read file " << path << " for unknown reasons :clown:";
    }
  }
  return file;
}

std::optional<Mesh> DataLoader::GetObject() {
  auto next_path = parser_.GetNext();
  if (!next_path) {
    return std::nullopt;
  }

  std::filesystem::path object_path = next_path.value();
  std::ifstream object_stream = OpenFile(object_path);
  ObjParser obj_parser = object_stream;
  return obj_parser.ConstructMesh();
}

std::vector<Mesh> DataLoader::GetAllObjects() {
  std::vector<Mesh> result;
  std::optional<Mesh> object = GetObject();
  while (object) {
    result.emplace_back(std::move(object.value()));
    object = GetObject();
  }
  return result;
}

}  // namespace renderer
