#pragma once

#include <fstream>
#include <glog/logging.h>

#include <scene/mesh.hpp>

namespace renderer {

std::optional<Mesh> LoadMeshFromObjFile(const std::string& filepath);

}