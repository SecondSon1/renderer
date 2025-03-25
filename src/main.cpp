#include <imgui.h>
#include <glog/logging.h>
#include "fs/root_finder.hpp"
#include "app.hpp"
#include "except.hpp"

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

}  // namespace

int main(int argc, char *argv[]) {
  InitializeGlobalObjects(argv[0]);

  renderer::MoveToRoot();

  try {
    renderer::Application app;
    app.Run();
  } catch (...) {
    except::react();
  }

  return 0;
}
