#include "view.hpp"

#include <cmath>
#include <cstdint>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include <glog/logging.h>

namespace renderer {

namespace {

constexpr static auto kWindowFlags =
    SDL::Window::Flags::kInputFocus | SDL::Window::Flags::kMouseFocus;

}

View::View(Width width, Height height, std::string title)
    : window_(title.data(), width, height, kWindowFlags), renderer_(window_, nullptr) {
}

void View::Display(const renderer::ImageWithDepth& image) {
  EndOptionsWindow();
  renderer_.Render(image);
  renderer_.Present();
}

void View::BeginFrame() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  options_drawn_ = false;
}

void View::DrawWireframeOption(bool& wireframe) {
  StartOptionsWindow();
  ImGui::Checkbox("Wireframe", &wireframe);
}

void View::DrawControllerOptions(Controller& ctl) {
  StartOptionsWindow();
  constexpr double kCameraSpeedMin = 1.0;
  constexpr double kCameraSpeedMax = 15.0;
  ImGui::SliderScalar("Camera speed", ImGuiDataType_Double, ctl.GetCameraSpeedPtr(),
                      &kCameraSpeedMin, &kCameraSpeedMax, "%.1lf");
  constexpr double kCameraRotSpeedMin = 0.1;
  constexpr double kCameraRotSpeedMax = 2;
  ImGui::SliderScalar("Camera rotation speed", ImGuiDataType_Double,
                      ctl.GetCameraRotationSpeedPtr(), &kCameraRotSpeedMin, &kCameraRotSpeedMax,
                      "%.2lf");
}

void View::DrawCameraOptions(Camera& camera) {
  StartOptionsWindow();

  const auto& cam_pos = camera.pos_;
  ImGui::Text("Camera position: x=%.2lf, y=%.2lf, z=%.2lf", cam_pos.x(), cam_pos.y(), cam_pos.z());
  ImGui::Text("Camera rotation as quaternion:");
  const auto& cam_dir = camera.dir_;
  ImGui::Text("  %.3lf + %.3lfi + %.3lfj + %.3lfk", cam_dir.w(), cam_dir.x(), cam_dir.y(),
              cam_dir.z());

  constexpr double kFOVMinVal = 30;
  constexpr double kFOVMaxVal = 150;
  ImGui::SliderScalar("FOV (deg)", ImGuiDataType_Double, &camera.fov_degrees_, &kFOVMinVal,
                      &kFOVMaxVal, "%.1lf");
  constexpr double kZNearMin = 0.001;
  constexpr double kZNearMax = 0.1;
  ImGui::SliderScalar("z_near", ImGuiDataType_Double, &camera.z_near_, &kZNearMin, &kZNearMax,
                      "%.3lf", ImGuiSliderFlags_Logarithmic);

  ImGui::Checkbox("z_far = inf", &camera.inf_z_far_);

  if (!camera.inf_z_far_) {
    constexpr double kZFarMin = 5.0;
    constexpr double kZFarMax = 10000.0;
    ImGui::SliderScalar("z_far", ImGuiDataType_Double, &camera.z_far_, &kZFarMin, &kZFarMax,
                        "%.1lf", ImGuiSliderFlags_Logarithmic);
  }
}

void View::DrawFPS(util::Timer::SecondsUnit avg_frame_time) {
  StartOptionsWindow();

  constexpr util::Timer::SecondsUnit kMillisInSecond = 1000.0;
  constexpr util::Timer::SecondsUnit kMinFrameTime = 0.001;
  ImGui::Text("Avg. frame time: %.1f ms", avg_frame_time * kMillisInSecond);
  if (avg_frame_time > kMinFrameTime) {
    uint32_t fps = std::round(1 / avg_frame_time);
    ImGui::Text("Avg. FPS: %d", fps);
  }
}

void View::DrawPauseOption(bool& paused) {
  StartOptionsWindow();
  ImGui::Checkbox("Pause", &paused);
}

const SDL::Window& View::GetWindow() const {
  return window_;
}

const SDL::Renderer& View::GetRenderer() const {
  return renderer_;
}

void View::StartOptionsWindow() {
  if (options_drawn_) {
    return;
  }
  ImGui::Begin("Options");
  options_drawn_ = true;
}

void View::EndOptionsWindow() const {
  if (options_drawn_) {
    ImGui::End();
  }
}

}  // namespace renderer
