#include "app.hpp"

#include <string_view>
#include <utility>
#include <vector>
#include <string>
#include <filesystem>
#include <glog/logging.h>
#include "fs/loader.hpp"
#include "scene/mesh.hpp"

namespace renderer {

namespace {

constexpr Width kWindowWidth = Width{1280};
constexpr Height kWindowHeight = Height{720};
constexpr std::string_view kWindowTitle = "3D Renderer";

}  // namespace

renderer::Application::Application(SceneCreator&& scene_creator)
    : renderer_(kWindowWidth, kWindowHeight),
      scene_(LoadScene(std::move(scene_creator))),
      last_image_(kWindowWidth, kWindowHeight),
      camera_(InitializeCamera()),
      ctl_(camera_),
      view_(kWindowWidth, kWindowHeight, std::string(kWindowTitle)) {
}

void Application::Run() {
  constexpr size_t kTargetFPS = 200;
  constexpr util::Timer::SecondsUnit kSecondsPerFrame = 1.0 / kTargetFPS;

  is_running_ = true;
  timer_.Reset();
  while (is_running_) {
    std::optional<SDL::Event> evt;
    while ((evt = SDL::PollEvent())) {
      HandleEvent(std::move(evt.value()));
    }

    view_.BeginFrame();
    auto seconds_per_last_frame = timer_.Lap();
    frame_timings_measurer_.AddTiming(seconds_per_last_frame);
    if (!is_paused_) {
      scene_->Advance(seconds_per_last_frame);
    }
    ctl_.Advance(seconds_per_last_frame);

    last_image_ = RenderImage();
    DrawOptions();

    view_.Display(last_image_);
    timer_.WaitUntilLapIs(kSecondsPerFrame);
  }
}

namespace {

constexpr std::string_view kDatalistFileName = "data/datalist.txt";

}

std::unique_ptr<Scene> Application::LoadScene(SceneCreator&& scene_creator) const {
  static const std::filesystem::path kDatalistFilePath = kDatalistFileName;
  DataLoader loader(kDatalistFilePath);
  std::vector<Mesh> objects = loader.GetAllObjects();

  std::string path_to_tex = "data/tex_all.jpg";
  std::filesystem::path fs_path(path_to_tex);
  Texture texture = TextureReader::LoadImage(fs_path);

  return scene_creator(std::move(objects), std::move(texture));
}

Camera Application::InitializeCamera() const {
  return {
      .fov_degrees_ = 90.0,
      .z_near_ = 0.01,
      .z_far_ = 1000.0,
      .inf_z_far_ = true,
  };
}

void Application::HandleEvent(SDL::Event&& event) {
  switch (event.type) {
    case SDL_EVENT_QUIT: {
      Quit();
    } break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
      if (event.window.windowID == view_.GetWindow().GetID()) {
        Quit();
      }
    } break;
    case SDL_EVENT_KEY_DOWN: {
      ctl_.KeyPressed(event.key);
    } break;
    case SDL_EVENT_KEY_UP: {
      ctl_.KeyReleased(event.key);
    } break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      ctl_.MousePressed(event.button);
    } break;
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      ctl_.MouseReleased(event.button);
    } break;
    case SDL_EVENT_MOUSE_MOTION: {
      ctl_.MouseMoved(event.motion);
    } break;
    case SDL_EVENT_MOUSE_WHEEL: {
      ctl_.MouseWheelMoved(event.wheel);
    } break;
  }
}

ImageWithDepth Application::RenderImage() const {
  if (is_wireframe_) {
    return renderer_.RenderWireframe(scene_.get(), camera_);
  } else {
    return renderer_.Render(scene_.get(), camera_);
  }
}

void Application::DrawOptions() {
  view_.DrawFPS(frame_timings_measurer_.GetAverageTiming());
  view_.DrawPauseOption(is_paused_);
  view_.DrawWireframeOption(is_wireframe_);
  view_.DrawControllerOptions(ctl_);
  view_.DrawCameraOptions(camera_);
}

void Application::Quit() {
  is_running_ = false;
}

}  // namespace renderer
