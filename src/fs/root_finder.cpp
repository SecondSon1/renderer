#include "fs/root_finder.hpp"

#include <filesystem>
#include <string_view>
#include <glog/logging.h>

namespace renderer {

namespace fs = std::filesystem;

namespace {

constexpr std::string_view kRootIndicatorFile = ".renderer-root";

bool DoesIndicatorExistAt(fs::path path) {
  static const fs::path indicator = kRootIndicatorFile;
  path /= indicator;
  return fs::exists(path);
}

}

void MoveToRoot() noexcept {
  fs::path path = fs::current_path();
  while (!DoesIndicatorExistAt(path)) {
    fs::path parent = path.parent_path();
    if (parent == path) {
      LOG(FATAL) << "Unable to find project root directory";
    }
    path = parent;
  }
  LOG(INFO) << "Project's root directory is \"" << path << "\", switching there now";
  fs::current_path(path);
}

}  // namespace renderer
