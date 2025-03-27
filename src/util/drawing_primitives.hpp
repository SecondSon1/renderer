#pragma once

#include <utility>
#include "graphics/image.hpp"
#include "scene/mesh.hpp"

namespace renderer {

namespace util {

struct IndexWithDepth : Index {
  float z_;
};

void DrawLine(ImageWithDepth& img, Index from, Index to, Pixel color);
void FillTriangle(ImageWithDepth& img, IndexWithDepth v1, IndexWithDepth v2, IndexWithDepth v3, Pixel color);

}  // namespace util

}  // namespace renderer
