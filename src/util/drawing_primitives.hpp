#pragma once

#include "graphics/image.hpp"
#include "scene/mesh.hpp"

namespace renderer {

namespace util {

void DrawLine(Image& img, Index from, Index to, Pixel color);
void FillTriangle(Image& img, Index v1, Index v2, Index v3, Pixel color);

}  // namespace util

}  // namespace renderer
