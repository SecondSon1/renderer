#include <SDL3/SDL.h>
#include <glog/logging.h>

#include <sdl_wrap.hpp>

constexpr size_t WINDOW_WIDTH = 1280;
constexpr size_t WINDOW_HEIGHT = 720;
const char *WINDOW_TITLE = "3D Renderer";

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);

  SDL::Window window(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  SDL::Renderer renderer = window.GetRenderer(nullptr);

  SDL_SetRenderDrawColor(renderer.renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer.renderer);

  LOG(INFO) << "Started drawing";
  // TODO: Rendering here :)
  LOG(INFO) << "Finished drawing";

  SDL_RenderPresent(renderer.renderer);

  bool shouldClose = false;
  while (!shouldClose) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        shouldClose = true;
        break;
      }
    }
  }

  return 0;
}