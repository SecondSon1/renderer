#include <utility>
#include <imgui.h>
#include <glog/logging.h>
#include "fs/root_finder.hpp"
#include "app.hpp"
#include "except.hpp"
#include "scenes/example_scene.hpp"

namespace {

// https://github.com/ocornut/imgui/wiki/Getting-Started
void InitializeDearImgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();
}

void InitializeGlobalObjects(const char *argv0) {
  google::InitGoogleLogging(argv0);
  InitializeDearImgui();
}

renderer::ScenePtr CreateExampleScene(std::vector<renderer::Mesh> &&mesh, renderer::Texture &&tex) {
  return std::make_unique<renderer::example::ExampleScene>(std::move(mesh), std::move(tex));
}

}  // namespace

int main(int argc, char *argv[]) {
  InitializeGlobalObjects(argv[0]);

  renderer::MoveToRoot();

  try {
    renderer::Application app(CreateExampleScene);
    app.Run();
  } catch (...) {
    except::react();
  }

  return 0;
}
