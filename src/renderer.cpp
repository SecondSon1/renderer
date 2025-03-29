#include "renderer.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <utility>
#include <variant>
#include <vector>
#include <Eigen/Core>
#include <glog/logging.h>
#include "util/drawing_primitives.hpp"
#include "util/util.hpp"
#include "linalg.hpp"
#include "scene/mesh.hpp"
#include "scene/lighting.hpp"
#include "graphics/image.hpp"

namespace renderer {

namespace {

template <size_t kMaxAmount>
struct ClippedResult {
  size_t count_;
  std::array<Triangle, kMaxAmount> tris_;

  template <size_t kLargerMaxAmount>
    requires(kLargerMaxAmount > kMaxAmount)
  operator ClippedResult<kLargerMaxAmount>() const {
    ClippedResult<kLargerMaxAmount> result = {
        .count_ = count_,
    };
    std::copy(tris_.begin(), tris_.begin() + count_, result.tris_.begin());
    return result;
  }
};

Vector3d IntersectPlaneAndLine(Vector3d plane_norm, double d, Vector3d line_start,
                               Vector3d line_dir) {
  double num = -(plane_norm.dot(line_start) + d);
  double den = plane_norm.dot(line_dir);
  double t = util::Sign(den) == 0 ? 0 : (num / den);
  return line_start + line_dir * t;
}

Triangle ClipSingleResult(Vector3d plane_norm, double d, Vector3d inside, Vector3d outside1,
                          Vector3d outside2) {
  Triangle result{};
  result[0] = inside;
  result[1] = IntersectPlaneAndLine(plane_norm, d, inside, outside1 - inside);
  result[2] = IntersectPlaneAndLine(plane_norm, d, inside, outside2 - inside);
  return result;
}

std::pair<Triangle, Triangle> ClipTwoResults(Vector3d plane_norm, double d, Vector3d inside1,
                                             Vector3d inside2, Vector3d outside) {
  Vector3d intersect1 = IntersectPlaneAndLine(plane_norm, d, outside, inside1 - outside);
  Vector3d intersect2 = IntersectPlaneAndLine(plane_norm, d, outside, inside2 - outside);

  Triangle result1{};
  result1[0] = inside1;
  result1[1] = inside2;
  result1[2] = intersect1;

  Triangle result2{};
  result2[0] = intersect1;
  result2[1] = inside2;
  result2[2] = intersect2;
  return {std::move(result1), std::move(result2)};
}

ClippedResult<2> ClipAgainstPlane(const Triangle& tri, Vector3d plane_norm, double d) {
  assert(util::AlmostEqual(plane_norm.squaredNorm(), 1.0));

  uint8_t count_positive = 0;
  uint8_t positive_inds[3] = {0};
  uint8_t count_negative = 0;
  uint8_t negative_inds[3] = {0};

  for (uint8_t i = 0; i < 3; ++i) {
    if (util::Sign(plane_norm.dot(tri[i]) + d) > 0) {
      positive_inds[count_positive++] = i;
    } else {
      negative_inds[count_negative++] = i;
    }
  }

  assert(count_negative + count_positive == 3);
  ClippedResult<2> result{};
  if (count_positive == 0) {
    return result;
  }
  if (count_positive == 3) {
    result.count_ = 1;
    result.tris_[0] = tri;
    return result;
  }

  if (count_positive == 1) {
    if (positive_inds[0] == 1) {
      std::swap(negative_inds[0], negative_inds[1]);
    }

    result.count_ = 1;
    auto clipped = ClipSingleResult(plane_norm, d, tri[positive_inds[0]], tri[negative_inds[0]],
                                    tri[negative_inds[1]]);
    clipped.color_ = tri.color_;
    result.tris_[0] = std::move(clipped);
  } else {
    if (negative_inds[0] == 1) {
      std::swap(positive_inds[0], positive_inds[1]);
    }

    result.count_ = 2;
    auto [clipped1, clipped2] = ClipTwoResults(plane_norm, d, tri[positive_inds[0]],
                                               tri[positive_inds[1]], tri[negative_inds[0]]);
    clipped1.color_ = tri.color_;
    clipped2.color_ = tri.color_;
    result.tris_[0] = std::move(clipped1);
    result.tris_[1] = std::move(clipped2);
  }
  return result;
}

ClippedResult<4> ClipAgainstTwoPlanes(const Triangle& tri, Vector3d plane1_norm, double d1,
                                      Vector3d plane2_norm, double d2) {
  ClippedResult<2> against1 = ClipAgainstPlane(tri, plane1_norm, d1);
  ClippedResult<4> result;

  result.count_ = 0;
  for (size_t i = 0; i < against1.count_; ++i) {
    ClippedResult<2> against_far = ClipAgainstPlane(against1.tris_[i], plane2_norm, d2);
    for (size_t j = 0; j < against_far.count_; ++j) {
      size_t cur_idx = result.count_++;
      result.tris_[cur_idx] = against_far.tris_[j];
    }
  }
  return result;
}

ClippedResult<4> ClipAgainstNearAndFar(const Triangle& tri, const Camera& camera) {
  static const Vector3d near_plane_normal(0.0, 0.0, -1.0);
  const double near_plane_d = -camera.z_near_;
  static const Vector3d far_plane_normal(0.0, 0.0, 1.0);
  const double far_plane_d = camera.z_far_;

  if (camera.inf_z_far_) {
    return ClipAgainstPlane(tri, near_plane_normal, near_plane_d);
  } else {
    return ClipAgainstTwoPlanes(tri, near_plane_normal, near_plane_d, far_plane_normal,
                                far_plane_d);
  }
}

ClippedResult<4> ClipAgainstLeftAndRight(const Triangle& tri, const Camera& camera) {
  const double e = camera.ComputeDistToScreen();
  const double inv_len = std::sqrtl(1.0 / (e * e + 1));

  const Vector3d left_plane_normal(e * inv_len, 0.0, -inv_len);
  const Vector3d right_plane_normal(-e * inv_len, 0.0, -inv_len);

  return ClipAgainstTwoPlanes(tri, left_plane_normal, 0, right_plane_normal, 0);
}

ClippedResult<4> ClipAgainstTopAndBottom(const Triangle& tri, const Camera& camera,
                                         double aspect_ratio) {
  const double e = camera.ComputeDistToScreen();
  const double inv_len = std::sqrt(1.0 / (e * e + aspect_ratio * aspect_ratio));

  const Vector3d bottom_plane_normal(0.0, e * inv_len, -aspect_ratio * inv_len);
  const Vector3d top_plane_normal(0.0, -e * inv_len, -aspect_ratio * inv_len);

  return ClipAgainstTwoPlanes(tri, bottom_plane_normal, 0, top_plane_normal, 0);
}

std::vector<Triangle> ClipTriangles(const std::vector<Triangle>& mesh, const Camera& camera,
                                    double aspect_ratio) {
  std::vector<Triangle> against_near_and_far;
  against_near_and_far.reserve(mesh.size() * 2 * 2);
  for (const Triangle& tri : mesh) {
    ClippedResult<4> clipped = ClipAgainstNearAndFar(tri, camera);
    for (size_t i = 0; i < clipped.count_; ++i) {
      against_near_and_far.emplace_back(std::move(clipped.tris_[i]));
    }
  }
  std::vector<Triangle> against_left_and_right;
  against_left_and_right.reserve(against_near_and_far.size() * 2 * 2);
  for (const Triangle& tri : against_near_and_far) {
    ClippedResult<4> clipped = ClipAgainstLeftAndRight(tri, camera);
    for (size_t i = 0; i < clipped.count_; ++i) {
      against_left_and_right.emplace_back(std::move(clipped.tris_[i]));
    }
  }
  std::vector<Triangle> against_top_and_bottom;
  against_top_and_bottom.reserve(against_left_and_right.size() * 2 * 2);
  for (const Triangle& tri : against_left_and_right) {
    ClippedResult<4> clipped = ClipAgainstTopAndBottom(tri, camera, aspect_ratio);
    for (size_t i = 0; i < clipped.count_; ++i) {
      against_top_and_bottom.emplace_back(std::move(clipped.tris_[i]));
    }
  }
  return against_top_and_bottom;
}

bool CullingTest(const Triangle& tri, const Camera& camera) {
  Vector3d look_dir = tri[0];
  return look_dir.dot(tri.GetNonUnitNormal()) < 0;
}

HDRPixel CalculateLightColor(const Triangle& tri, const DirectionalLight& directional,
                             float normalized_intensity) {
  assert(util::AlmostEqual(directional.direction_.squaredNorm(), 1));
  Vector3d tri_normal = tri.GetNonUnitNormal();
  tri_normal.normalize();
  double dot = tri_normal.dot(directional.direction_);
  float blending_coef = std::max(-dot, 0.0);
  return directional.color_ * (normalized_intensity * blending_coef);
}

HDRPixel CalculateLightColor(const Triangle& tri, const Lighting& lighting) {
  double intensity = 0;
  HDRPixel result{};
  for (const LightSource& light_source : lighting) {
    float cur_intensity = light_source.GetBase().intensity_;
    assert(util::Sign(cur_intensity) > 0);
    intensity += cur_intensity;
  }
  assert(util::Sign(intensity) > 0);
  const double intensity_inv = 1 / intensity;

  auto handlers = util::overloaded{
      [&result, intensity_inv](const AmbientLight& ambient) {
        result += ambient.color_ * (ambient.intensity_ * intensity_inv);
      },
      [&result, &tri, intensity_inv](const DirectionalLight& directional) {
        result += CalculateLightColor(tri, directional, directional.intensity_ * intensity_inv);
      },
      [](const PointLightSource& point) { LOG(FATAL) << "Point light not supported yet"; },
  };
  for (const LightSource& light_source : lighting) {
    std::visit(handlers, light_source);
  }
  return result;
}

}  // namespace

Renderer::Renderer(Width screen_width, Height screen_height)
    : screen_width_(screen_width), screen_height_(screen_height) {
  assert(screen_width_ > 0);
  assert(screen_height_ > 0);
}

ImageWithDepth Renderer::Render(const Scene* scene, const Camera& camera) {
  double aspect_ratio = static_cast<double>(screen_height_) / screen_width_;
  auto proj_mat = camera.GetProjMatrix(aspect_ratio);
  ImageWithDepth result(static_cast<Width>(screen_width_), static_cast<Height>(screen_height_));

  Mat4x4d world_to_camera = camera.GetWorldToCameraTransform();
  Lighting lighting = scene->GetLighting();

  for (const Mesh& mesh : scene->GetMeshes()) {
    std::vector<Triangle> tris_to_clip;

    for (const Triangle& tri : mesh.triangles_) {
      Triangle res = tri;
      res += mesh.local_zero_;
      res.color_ = static_cast<Pixel>(CalculateLightColor(res, lighting));
      res = res.Transform(world_to_camera);
      if (CullingTest(res, camera)) {
        tris_to_clip.emplace_back(std::move(res));
      }
    }
    std::vector<Triangle> clipped_triangles = ClipTriangles(tris_to_clip, camera, aspect_ratio);
    for (const Triangle& tri : clipped_triangles) {
      auto tri_projected = tri.Transform(proj_mat);
      FillTriangle(result, tri_projected, tri.color_);
    }
  }
  return result;
}

ImageWithDepth Renderer::RenderWireframe(const Scene* scene, const Camera& camera) {
  double aspect_ratio = static_cast<double>(screen_height_) / screen_width_;
  auto proj_mat = camera.GetProjMatrix(aspect_ratio);
  ImageWithDepth result(static_cast<Width>(screen_width_), static_cast<Height>(screen_height_));

  Mat4x4d world_to_camera = camera.GetWorldToCameraTransform();
  for (Mesh mesh : scene->GetMeshes()) {
    for (Triangle& tri : mesh.triangles_) {
      tri += mesh.local_zero_;
      tri = tri.Transform(world_to_camera);
    }
    std::vector<Triangle> clipped_triangles = ClipTriangles(mesh.triangles_, camera, aspect_ratio);
    for (const Triangle& tri : clipped_triangles) {
      auto tri_projected = tri.Transform(proj_mat);
      DrawTriangle(result, tri_projected);
    }
  }
  return result;
}

Index Renderer::NormalizedToIndex(Vector2d vec) const {
  double x = vec[0];
  double y = vec[1];
  assert(util::Sign(x + 1) >= 0 && util::Sign(x - 1) <= 0);
  assert(util::Sign(y + 1) >= 0 && util::Sign(y - 1) <= 0);
  double x_mult = (x + 1) / 2 * (screen_width_ - 1);
  double y_mult = (y + 1) / 2 * (screen_height_ - 1);
  size_t x_ind = std::round(x_mult);
  size_t y_ind = std::round(y_mult);
  return Col(x_ind), Row(y_ind);
}

void Renderer::DrawTriangle(ImageWithDepth& img, const Triangle& tri) const {
  Index v0 = NormalizedToIndex(tri[0].head<2>());
  Index v1 = NormalizedToIndex(tri[1].head<2>());
  Index v2 = NormalizedToIndex(tri[2].head<2>());

  util::DrawLine(img, v0, v1, colors::kWhite);
  util::DrawLine(img, v0, v2, colors::kWhite);
  util::DrawLine(img, v1, v2, colors::kWhite);
}

void Renderer::FillTriangle(ImageWithDepth& img, const Triangle& tri, Pixel color) const {
  Index v0 = NormalizedToIndex(tri[0].head<2>());
  Index v1 = NormalizedToIndex(tri[1].head<2>());
  Index v2 = NormalizedToIndex(tri[2].head<2>());

  util::IndexWithDepth v0d = {v0, static_cast<float>(tri[0][2])};
  util::IndexWithDepth v1d = {v1, static_cast<float>(tri[1][2])};
  util::IndexWithDepth v2d = {v2, static_cast<float>(tri[2][2])};

  util::FillTriangle(img, v0d, v1d, v2d, color);
}

}  // namespace renderer
