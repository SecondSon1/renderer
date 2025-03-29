#pragma once

#include <memory>
#include "linalg.hpp"
#include "scene/scene.hpp"
#include "scene/camera.hpp"
#include "scene/mesh.hpp"
#include "graphics/image.hpp"
#include "fs/texture_reader.hpp"

namespace renderer {

class Renderer {
 public:
  Renderer(Width screen_width, Height screen_height);

  ImageWithDepth Render(const Scene* scene, const Camera& camera);
  ImageWithDepth RenderWireframe(const Scene* scene, const Camera& camera);

 private:
  Index NormalizedToIndex(Vector2d vec) const;
  void DrawTriangle(ImageWithDepth& img, const Triangle& tri) const;
  void FillTriangle(ImageWithDepth& img, const Triangle& tri, const Texture& tex, Pixel lighting_color) const;

 private:
  size_t screen_width_;
  size_t screen_height_;
};

}  // namespace renderer
