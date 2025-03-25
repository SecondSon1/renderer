#include "view.hpp"

#include <cmath>
#include <cstdint>
#include <numbers>
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

void View::Display(const renderer::Image& image) {
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

void View::DrawCameraOptions(Camera& camera) {
  StartOptionsWindow();
  constexpr double kFOVMinVal = std::numbers::pi / 6;
  constexpr double kFOVMaxVal = std::numbers::pi * 2 / 3;
  ImGui::SliderScalar("FOV (rad)", ImGuiDataType_Double, &camera.fov, &kFOVMinVal,
                      &kFOVMaxVal, "%.1lf");
  constexpr double kZNearMin = 0.001;
  constexpr double kZNearMax = 0.1;
  ImGui::SliderScalar("z_near", ImGuiDataType_Double, &camera.z_near, &kZNearMin, &kZNearMax,
                      "%.3lf", ImGuiSliderFlags_Logarithmic);
  constexpr double kZFarMin = 10.0;
  constexpr double kZFarMax = 10000.0;
  ImGui::SliderScalar("z_far", ImGuiDataType_Double, &camera.z_far, &kZFarMin, &kZFarMax, "%.1lf",
                      ImGuiSliderFlags_Logarithmic);
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
