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

namespace renderer {

namespace {

constexpr Width kWindowWidth = Width{1280};
constexpr Height kWindowHeight = Height{720};
constexpr std::string_view kWindowTitle = "3D Renderer";

}  // namespace

renderer::Application::Application()
    : renderer_(kWindowWidth, kWindowHeight),
      scene_(LoadScene()),
      camera_(InitializeCamera()),
      view_(kWindowWidth, kWindowHeight, std::string(kWindowTitle)) {
}

void Application::Run() {
  constexpr size_t kTargetFPS = 30;
  constexpr util::Timer::SecondsUnit kSecondsPerFrame = 1.0 / kTargetFPS;

  timer_.Reset();
  while (true) {
    SDL_Event evt;
    while (SDL_PollEvent(&evt)) {
      if (evt.type == SDL_EVENT_QUIT) {
        return;
      }
      if (evt.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          evt.window.windowID == view_.GetWindow().GetID()) {
        return;
      }
    }
    scene_.Advance(timer_.Lap());
    Image img = renderer_.RenderWireframe(scene_, camera_);
    view_.Display(img);
    timer_.WaitUntilLapIs(kSecondsPerFrame);
  }
}

namespace {

constexpr std::string_view kDatalistFileName = "data/datalist.txt";

}

Scene Application::LoadScene() const {
  static std::filesystem::path kDatalistFilePath = kDatalistFileName;

  DataLoader loader(kDatalistFilePath);
  std::vector<Mesh> objects = loader.GetAllObjects();
  // temp
  objects[0].local_zero_(2) = -4;
  return {std::move(objects)};
}

Camera Application::InitializeCamera() const {
  return {
      .fov = std::numbers::pi / 2,
      .z_near = 0.01,
      .z_far = 1000.0,
  };
}

}  // namespace renderer
