#pragma once

#include "linalg.hpp"
#include "scene/scene.hpp"
#include "scene/camera.hpp"
#include "scene/mesh.hpp"
#include "graphics/image.hpp"
#include "util/drawing_primitives.hpp"

namespace renderer {

class Renderer {
 public:
  Renderer(Width screen_width, Height screen_height);

  ImageWithDepth Render(const Scene* scene, const Camera& camera) const;
  ImageWithDepth RenderWireframe(const Scene* scene, const Camera& camera) const;

 private:
  Index NormalizedToIndex(Vector2d vec) const;
  void DrawTriangle(util::Rasterizer& rst, const Triangle& tri, Pixel color) const;
  void FillTriangle(util::Rasterizer& rst, const Triangle& tri, Pixel lighting_color) const;

 private:
  size_t screen_width_;
  size_t screen_height_;
};

}  // namespace renderer
