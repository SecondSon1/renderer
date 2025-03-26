#pragma once

#include <memory>
#include "linalg.hpp"
#include "scene/scene.hpp"
#include "scene/camera.hpp"
#include "scene/mesh.hpp"
#include "graphics/image.hpp"

namespace renderer {

class Renderer {
 public:
  Renderer(Width screen_width, Height screen_height);

  Image Render(const Scene* scene, const Camera& camera);
  Image RenderWireframe(const Scene* scene, const Camera& camera);

 private:
  Index NormalizedToIndex(Vector2d vec) const;
  void DrawTriangle(Image& img, Triangle tri) const;

 private:
  size_t screen_width_;
  size_t screen_height_;
};

}  // namespace renderer
