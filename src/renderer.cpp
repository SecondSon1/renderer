#include "renderer.hpp"

#include <cstdint>
#include <utility>
#include <cassert>
#include <cmath>
#include <Eigen/Core>
#include <glog/logging.h>

namespace renderer {

namespace {

/*
void DrawVerticalLine(Image& img, size_t x, size_t y1, size_t y2, Pixel color) {
  if (y1 > y2) {
    std::swap(y1, y2);
  }
  for (size_t y = y1; y < y2; ++y) {
    img[Col{x}, Row{y}] = color;
  }
}
*/

void DrawLine(Image& img, Index from, Index to, Pixel color) {
  assert(0 <= from.col_ && from.col_ < img.GetWidth());
  assert(0 <= to.col_ && to.col_ < img.GetWidth());
  assert(0 <= from.row_ && from.row_ < img.GetHeight());
  assert(0 <= to.row_ && to.row_ < img.GetHeight());
  if (from.col_ > to.col_) {
    std::swap(from, to);
  }
  int32_t x = from.col_;
  int32_t y = from.row_;
  int32_t x1 = to.col_;
  int32_t y1 = to.row_;
  int32_t y_dir = (y < y1 ? 1 : -1);
  int32_t dx = std::abs(x1 - x);
  int32_t dy = std::abs(y1 - y);
  int32_t error = dx - dy;
  img[Col(x), Row(y)] = color;

  while (!(x == x1 && y == y1)) {
    auto err2 = error * 2;
    if (err2 > -dy) {
      error -= dy;
      ++x;
    }
    if (err2 < dx) {
      error += dx;
      y += y_dir;
    }
    img[Col(x), Row(y)] = color;
  }
}

}  // namespace

Renderer::Renderer(Width screen_width, Height screen_height)
    : screen_width_(screen_width), screen_height_(screen_height) {
  assert(screen_width_ > 0);
  assert(screen_height_ > 0);
}

Image Renderer::Render(const Scene& scene, const Camera& camera) {
  // TODO
  return RenderWireframe(scene, camera);
}

Image Renderer::RenderWireframe(const Scene& scene, const Camera& camera) {
  // TODO: clipping
  double aspect_ratio = static_cast<double>(screen_height_) / screen_width_;
  auto proj_mat = camera.GetProjMatrix(aspect_ratio);
  Image result = Image(Width(screen_width_), Height(screen_height_));

  // SDL_SetRenderDrawColor(sdlRenderer.renderer, 255, 255, 255, 255);
  for (const auto& mesh : scene.meshes_) {
    // auto projectedMesh = mesh.Transform(projMat);
    for (const auto& tri : mesh.triangles_) {
      auto tri_global_cds = tri + mesh.local_zero_;
      auto tri_projected = tri_global_cds.Transform(proj_mat);
      DrawTriangle(result, tri_projected);
    }
  }
  return result;
}

Index Renderer::NormalizedToIndex(Vector2d vec) const {
  double x = vec[0];
  double y = vec[1];
  assert(-1 <= x && x <= 1);
  assert(-1 <= y && y <= 1);
  double x_mult = (x + 1) / 2 * (screen_width_ - 1);
  double y_mult = (y + 1) / 2 * (screen_height_ - 1);
  size_t x_ind = std::round(x_mult);
  size_t y_ind = std::round(y_mult);
  return Col(x_ind), Row(y_ind);
}

void Renderer::DrawTriangle(Image& img, Triangle tri) const {
  Index v0 = NormalizedToIndex(tri[0].head<2>());
  Index v1 = NormalizedToIndex(tri[1].head<2>());
  Index v2 = NormalizedToIndex(tri[2].head<2>());

  DrawLine(img, v0, v1, colors::kWhite);
  DrawLine(img, v0, v2, colors::kWhite);
  DrawLine(img, v1, v2, colors::kWhite);
}

}  // namespace renderer
