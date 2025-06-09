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

struct CoordAndTex {
  Vector3d cds_;
  Vector2d tex_;
};

CoordAndTex IntersectPlaneAndLine(Vector3d plane_norm, double d, CoordAndTex line_start,
                                  CoordAndTex line_end) {
  Vector3d line_dir = line_end.cds_ - line_start.cds_;
  double num = -(plane_norm.dot(line_start.cds_) + d);
  double den = plane_norm.dot(line_dir);
  double t = util::Sign(den) == 0 ? 0 : (num / den);
  Vector3d res_cds = line_start.cds_ + line_dir * t;
  Vector2d tex_interpolated = (1 - t) * line_start.tex_ + t * line_end.tex_;
  return {res_cds, tex_interpolated};
}

Triangle ClipSingleResult(Vector3d plane_norm, double d, CoordAndTex inside, CoordAndTex outside1,
                          CoordAndTex outside2, bool has_tex) {
  Triangle result{};
  result[0] = inside.cds_;
  auto [vec1, tex1] = IntersectPlaneAndLine(plane_norm, d, inside, outside1);
  result[1] = vec1;
  auto [vec2, tex2] = IntersectPlaneAndLine(plane_norm, d, inside, outside2);
  result[2] = vec2;
  if (has_tex) {
    result.texture_vertices_ = std::array<Triangle::TexVector, 3>();
    auto& tex_vertices = result.texture_vertices_.value();
    tex_vertices[0] = inside.tex_;
    tex_vertices[1] = tex1;
    tex_vertices[2] = tex2;
  }
  return result;
}

std::pair<Triangle, Triangle> ClipTwoResults(Vector3d plane_norm, double d, CoordAndTex inside1,
                                             CoordAndTex inside2, CoordAndTex outside,
                                             bool has_tex) {
  auto [vec1, tex1] = IntersectPlaneAndLine(plane_norm, d, outside, inside1);
  auto [vec2, tex2] = IntersectPlaneAndLine(plane_norm, d, outside, inside2);

  Triangle result1{};
  result1[0] = inside1.cds_;
  result1[1] = inside2.cds_;
  result1[2] = vec1;

  Triangle result2{};
  result2[0] = vec1;
  result2[1] = inside2.cds_;
  result2[2] = vec2;

  if (has_tex) {
    result1.texture_vertices_ = std::array<Triangle::TexVector, 3>();
    result2.texture_vertices_ = std::array<Triangle::TexVector, 3>();

    auto& tex_vertices1 = result1.texture_vertices_.value();
    auto& tex_vertices2 = result2.texture_vertices_.value();

    tex_vertices1[0] = inside1.tex_;
    tex_vertices1[1] = inside2.tex_;
    tex_vertices1[2] = tex1;

    tex_vertices2[0] = tex1;
    tex_vertices2[1] = inside2.tex_;
    tex_vertices2[2] = tex2;
  }

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
    CoordAndTex inside = {tri[positive_inds[0]]};
    CoordAndTex outside1 = {tri[negative_inds[0]]};
    CoordAndTex outside2 = {tri[negative_inds[1]]};
    if (tri.texture_vertices_) {
      auto& texes = tri.texture_vertices_.value();
      inside.tex_ = texes[positive_inds[0]];
      outside1.tex_ = texes[negative_inds[0]];
      outside2.tex_ = texes[negative_inds[1]];
    }
    auto clipped = ClipSingleResult(plane_norm, d, inside, outside1, outside2,
                                    static_cast<bool>(tri.texture_vertices_));
    clipped.color_ = tri.color_;
    result.tris_[0] = std::move(clipped);
  } else {
    if (negative_inds[0] == 1) {
      std::swap(positive_inds[0], positive_inds[1]);
    }

    result.count_ = 2;
    CoordAndTex inside1 = {tri[positive_inds[0]]};
    CoordAndTex inside2 = {tri[positive_inds[1]]};
    CoordAndTex outside = {tri[negative_inds[0]]};
    if (tri.texture_vertices_) {
      auto& texes = tri.texture_vertices_.value();
      inside1.tex_ = texes[positive_inds[0]];
      inside2.tex_ = texes[positive_inds[1]];
      outside.tex_ = texes[negative_inds[0]];
    }
    auto [clipped1, clipped2] = ClipTwoResults(plane_norm, d, inside1, inside2, outside,
                                               static_cast<bool>(tri.texture_vertices_));
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

ImageWithDepth Renderer::Render(const Scene* scene, const Camera& camera) const {
  double aspect_ratio = static_cast<double>(screen_height_) / screen_width_;
  auto proj_mat = camera.GetProjMatrix(aspect_ratio);

  ImageWithDepth result(static_cast<Width>(screen_width_), static_cast<Height>(screen_height_));
  util::Rasterizer rasterizer(result, scene->GetTexture());

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
      FillTriangle(rasterizer, tri_projected, tri.color_);
    }
  }
  return result;
}

ImageWithDepth Renderer::RenderWireframe(const Scene* scene, const Camera& camera) const {
  double aspect_ratio = static_cast<double>(screen_height_) / screen_width_;
  auto proj_mat = camera.GetProjMatrix(aspect_ratio);

  ImageWithDepth result(static_cast<Width>(screen_width_), static_cast<Height>(screen_height_));
  util::Rasterizer rasterizer(result, scene->GetTexture());

  Mat4x4d world_to_camera = camera.GetWorldToCameraTransform();
  for (Mesh mesh : scene->GetMeshes()) {
    for (Triangle& tri : mesh.triangles_) {
      tri += mesh.local_zero_;
      tri = tri.Transform(world_to_camera);
    }
    std::vector<Triangle> clipped_triangles = ClipTriangles(mesh.triangles_, camera, aspect_ratio);
    for (const Triangle& tri : clipped_triangles) {
      auto tri_projected = tri.Transform(proj_mat);
      DrawTriangle(rasterizer, tri_projected, colors::kWhite);
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

void Renderer::DrawTriangle(util::Rasterizer& rst, const Triangle& tri, Pixel color) const {
  Index v0 = NormalizedToIndex(tri[0].head<2>());
  Index v1 = NormalizedToIndex(tri[1].head<2>());
  Index v2 = NormalizedToIndex(tri[2].head<2>());

  rst.DrawLine(v0, v1, color);
  rst.DrawLine(v0, v2, color);
  rst.DrawLine(v1, v2, color);
}

void Renderer::FillTriangle(util::Rasterizer& rst, const Triangle& tri,
                            Pixel lighting_color) const {
  Index v0 = NormalizedToIndex(tri[0].head<2>());
  Index v1 = NormalizedToIndex(tri[1].head<2>());
  Index v2 = NormalizedToIndex(tri[2].head<2>());

  util::IndexWithTex v0t = {v0};
  util::IndexWithTex v1t = {v1};
  util::IndexWithTex v2t = {v2};
  if (tri.texture_vertices_) {
    auto& tex_vert = tri.texture_vertices_.value();
    v0t.tex_ = tex_vert[0];
    v1t.tex_ = tex_vert[1];
    v2t.tex_ = tex_vert[2];
  }

  util::IndexWithDepth v0d = {v0t, static_cast<float>(tri[0][2])};
  util::IndexWithDepth v1d = {v1t, static_cast<float>(tri[1][2])};
  util::IndexWithDepth v2d = {v2t, static_cast<float>(tri[2][2])};

  if (tri.texture_vertices_) {
    rst.FillTriangleFromTexture(v0d, v1d, v2d, lighting_color);
  } else {
    rst.FillTriangleSolidColor(v0d, v1d, v2d, lighting_color);
  }
}

}  // namespace renderer
