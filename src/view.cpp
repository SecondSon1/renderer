#include "view.hpp"

namespace renderer {

namespace {

constexpr static auto kWindowFlags =
    SDL::Window::Flags::kInputFocus | SDL::Window::Flags::kMouseFocus;

}

View::View(Width width, Height height, std::string title)
    : window_(title.data(), width, height, kWindowFlags), renderer_(window_, nullptr) {
}

void View::Display(const renderer::Image& image) {
  renderer_.Render(image);
  renderer_.Present();
}

const SDL::Window& View::GetWindow() const {
  return window_;
}

const SDL::Renderer& View::GetRenderer() const {
  return renderer_;
}

}  // namespace renderer
