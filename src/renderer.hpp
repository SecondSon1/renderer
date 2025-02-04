#pragma once

#include <concepts>
#include <cstdint>
#include <utility>
#include <Eigen/Core>

#include <scene/scene.hpp>

namespace renderer {

template <typename ColorPixel>
  requires(std::invocable<ColorPixel, size_t, size_t, uint8_t, uint8_t, uint8_t>)
class Renderer {
 public:
  Renderer(size_t screen_width, size_t screen_height, ColorPixel&& color_pixel)
      : screen_width_(screen_width),
        screen_height_(screen_height),
        color_pixel_(std::move(color_pixel)) {
  }

  void Render(const Scene& scene, /* WILL REMOVE: */ SDL::Renderer& sdlRenderer) {
    // TODO: clipping
    double e = 1 / tan(scene.camera_.fov / 2);
    double aspect = static_cast<double>(screen_height_) / screen_width_;

    double x_scale = aspect * e;
    double y_scale = e;
    double x_translation = 0;  // since camera is fixed to (0, 0, 0)
    double y_translation = 0;

    double z_range_inverse = 1 / (scene.camera_.z_far - scene.camera_.z_near);
    double z_scale = -(scene.camera_.z_far + scene.camera_.z_near) * z_range_inverse;
    double z_offset = -2 * scene.camera_.z_near * scene.camera_.z_far * z_range_inverse;

    Eigen::Matrix4d projMat;
    projMat << x_scale, 0, x_translation, 0,
               0, y_scale, y_translation, 0,
               0, 0, z_scale, z_offset,
               0, 0, -1, 0;

    SDL_SetRenderDrawColor(sdlRenderer.renderer, 255, 255, 255, 255);
    for (auto& mesh : scene.meshes_) {
      auto projectedMesh = mesh.Transform(projMat);
      for (auto& tri : projectedMesh.triangles) {
        float x0 = (tri.pts[0][0] + 1) / 2 * screen_width_,
              y0 = (-tri.pts[0][1] + 1) / 2 * screen_height_;
        float x1 = (tri.pts[1][0] + 1) / 2 * screen_width_,
              y1 = (-tri.pts[1][1] + 1) / 2 * screen_height_;
        float x2 = (tri.pts[2][0] + 1) / 2 * screen_width_,
              y2 = (-tri.pts[2][1] + 1) / 2 * screen_height_;

        // This is cheating, this will all be removed once we're using
        //    color_pixel_(...) to draw pixel
        SDL_RenderLine(sdlRenderer.renderer, x0, y0, x1, y1);
        SDL_RenderLine(sdlRenderer.renderer, x0, y0, x2, y2);
        SDL_RenderLine(sdlRenderer.renderer, x1, y1, x2, y2);
      }
    }
  }

 private:
  size_t screen_width_, screen_height_;
  const ColorPixel color_pixel_;
};

}  // namespace renderer
