#include "app.hpp"

#include <numbers>
#include <string_view>
#include <utility>
#include <vector>
#include <string>
#include <filesystem>
#include <glog/logging.h>
#include "fs/loader.hpp"
#include "scene/mesh.hpp"
#include "scenes/example_scene.hpp"

namespace renderer {

namespace {

constexpr Width kWindowWidth = Width{1280};
constexpr Height kWindowHeight = Height{720};
constexpr std::string_view kWindowTitle = "3D Renderer";

}  // namespace

renderer::Application::Application()
    : renderer_(kWindowWidth, kWindowHeight),
      scene_(LoadScene()),
      last_image_(kWindowWidth, kWindowHeight),
      camera_(InitializeCamera()),
      view_(kWindowWidth, kWindowHeight, std::string(kWindowTitle)) {
}

void Application::Run() {
  constexpr size_t kTargetFPS = 35;
  constexpr util::Timer::SecondsUnit kSecondsPerFrame = 1.0 / kTargetFPS;

  is_running = true;
  timer_.Reset();
  while (is_running) {
    std::optional<SDL::Event> evt;
    while ((evt = SDL::PollEvent())) {
      HandleEvent(std::move(evt.value()));
    }

    view_.BeginFrame();
    auto seconds_per_last_frame = timer_.Lap();
    frame_timings_measurer_.AddTiming(seconds_per_last_frame);
    if (!is_paused) {
      scene_->Advance(seconds_per_last_frame);
    }

    RenderImage();
    DrawOptions();

    view_.Display(last_image_);
    timer_.WaitUntilLapIs(kSecondsPerFrame);
  }
}

namespace {

constexpr std::string_view kDatalistFileName = "data/datalist.txt";

}

std::unique_ptr<Scene> Application::LoadScene() const {
  static const std::filesystem::path kDatalistFilePath = kDatalistFileName;
  DataLoader loader(kDatalistFilePath);
  std::vector<Mesh> objects = loader.GetAllObjects();

  return std::make_unique<example::ExampleScene>(std::move(objects));
}

Camera Application::InitializeCamera() const {
  return {
      .fov_degrees_ = 90.0,
      .z_near_ = 1.0,
      .z_far_ = 1000.0,
      .inf_z_far_ = true,
  };
}

void Application::HandleEvent(SDL::Event&& event) {
  if (event.type == SDL_EVENT_QUIT) {
    is_running = false;
  }
  if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
      event.window.windowID == view_.GetWindow().GetID()) {
    is_running = false;
  }
}

void Application::RenderImage() {
  if (is_wireframe) {
    last_image_ = renderer_.RenderWireframe(scene_.get(), camera_);
  } else {
    last_image_ = renderer_.Render(scene_.get(), camera_);
  }
}

void Application::DrawOptions() {
  view_.DrawFPS(frame_timings_measurer_.GetAverageTiming());
  view_.DrawPauseOption(is_paused);
  view_.DrawWireframeOption(is_wireframe);
  view_.DrawCameraOptions(camera_);
}

}  // namespace renderer
