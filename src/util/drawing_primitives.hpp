#pragma once

#include "graphics/image.hpp"
#include "linalg.hpp"
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

class Rasterizer {
 public:
  Rasterizer(ImageWithDepth& img_to_draw_to, const Texture& main_texture);

  void DrawLine(Index from, Index to, Pixel color);
  void FillTriangleSolidColor(IndexWithDepth v1, IndexWithDepth v2, IndexWithDepth v3,
                              Pixel lighting_color);
  void FillTriangleFromTexture(IndexWithDepth v1, IndexWithDepth v2, IndexWithDepth v3,
                               Pixel lighting_color);

 private:
  ImageWithDepth* img_;
  const Texture* tex_;
};

}  // namespace util

}  // namespace renderer
