#pragma once

#include <utility>
#include "graphics/image.hpp"
#include "scene/mesh.hpp"
#include "fs/texture_reader.hpp"

namespace renderer {

namespace util {

struct IndexWithTex : Index {
  Vector2d tex_;
};

struct IndexWithDepth : IndexWithTex {
  float z_;

  IndexWithTex ToIndex() const {
    Index result_ind = {
        .row_ = row_,
        .col_ = col_,
    };
    return {result_ind, tex_};
  }
};

void DrawLine(ImageWithDepth& img, Index from, Index to, Pixel color);
void FillTriangle(ImageWithDepth& img, IndexWithDepth v1, IndexWithDepth v2, IndexWithDepth v3,
                  const Texture& tex, Pixel lighting_color, bool is_tex);

}  // namespace util

}  // namespace renderer
