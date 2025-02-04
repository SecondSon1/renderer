#include <SDL3/SDL.h>
#include <glog/logging.h>
#include <filesystem>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <numbers>
#include <chrono>
#include <cmath>

#include <sdl_wrap.hpp>
#include <renderer.hpp>
#include <scene/scene.hpp>
#include <scene/from_obj.hpp>

constexpr size_t WINDOW_WIDTH = 1280;
constexpr size_t WINDOW_HEIGHT = 720;
const char *WINDOW_TITLE = "3D Renderer";

const char *PATH_TO_PROJECT_ROOT = "../../../../";  // visual studio :clown:
const char *PATH_TO_CUBE = "data/cube.obj";

renderer::Mesh LoadCube() {
  auto cubeMesh = renderer::LoadMeshFromObjFile(PATH_TO_CUBE);
  if (!cubeMesh) {
    LOG(FATAL) << "could not load cube mesh";
  }
  return cubeMesh.value();
}

renderer::Mesh Transform(const renderer::Mesh &mesh, double elapsed) {
  Eigen::Matrix4d translationMatrix = Eigen::Matrix4d::Identity();
  // translation is because .obj cube is from (-1, -1, -1) to (1, 1, 1)
  // camera for now it permanently at (0, 0, 0)
  // we do not want to be inside the cube

  translationMatrix(0, 3) = 2 * std::sin(elapsed);
  translationMatrix(1, 3) = 0;
  translationMatrix(2, 3) = -4;

  Eigen::Matrix4d rotationMatrixX = Eigen::Matrix4d::Identity();
  rotationMatrixX(1, 1) = std::cos(elapsed * 2);
  rotationMatrixX(1, 2) = -std::sin(elapsed * 2);
  rotationMatrixX(2, 1) = std::sin(elapsed * 2);
  rotationMatrixX(2, 2) = std::cos(elapsed * 2);

  Eigen::Matrix4d rotationMatrixZ = Eigen::Matrix4d::Identity();
  rotationMatrixZ(0, 0) = std::cos(elapsed);
  rotationMatrixZ(0, 1) = -std::sin(elapsed);
  rotationMatrixZ(1, 0) = std::sin(elapsed);
  rotationMatrixZ(1, 1) = std::cos(elapsed);

  return mesh.Transform(translationMatrix * rotationMatrixX * rotationMatrixZ);
}

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = google::GLOG_INFO;  // for debugging

  std::filesystem::current_path(PATH_TO_PROJECT_ROOT);
  LOG(INFO) << "cwd: " << std::filesystem::current_path();

  SDL::Window window(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  SDL::Renderer sdlRenderer = window.GetRenderer(nullptr);

  auto color_pixel = [&sdlRenderer](size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b) {
    SDL_SetRenderDrawColor(sdlRenderer.renderer, r, g, b, 255);
    // SDL is top-to-bottom, so:
    y = WINDOW_HEIGHT - y;
    SDL_RenderPoint(sdlRenderer.renderer, x, y);
  };

  renderer::Renderer rendererObject(WINDOW_WIDTH, WINDOW_HEIGHT, std::move(color_pixel));

  // for now camera is fixed to (0, 0, 0) looking at (0, 0, -1)
  renderer::Camera camera(std::numbers::pi / 2, 0.1, 100.0);
  renderer::Mesh cubeMesh = LoadCube();

  auto start = std::chrono::high_resolution_clock::now();

  bool shouldClose = false;
  while (!shouldClose) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        shouldClose = true;
        break;
      }
    }

    SDL_SetRenderDrawColor(sdlRenderer.renderer, 0, 0, 0, 0);
    SDL_RenderClear(sdlRenderer.renderer);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto elapsedInSeconds =
        std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.0;

    renderer::Scene sceneToRender({Transform(cubeMesh, elapsedInSeconds)}, camera);
    rendererObject.Render(sceneToRender, sdlRenderer);

    SDL_RenderPresent(sdlRenderer.renderer);
    SDL_Delay(16);
  }

  return 0;
}