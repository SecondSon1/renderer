#pragma once

#include <array>
#include <initializer_list>
#include <memory>
#include "scene/camera.hpp"
#include "util/timer.hpp"
#include "linalg.hpp"
#include "graphics/sdl_wrap.hpp"

namespace renderer {

namespace Impl {

constexpr size_t kAmountOfScancodes = 256;

}

class Controller {
 public:
  Controller(Camera& camera);

  void Advance(util::Timer::SecondsUnit elapsed);

  void KeyPressed(SDL::KeyboardEvent kb_event);
  void KeyReleased(SDL::KeyboardEvent kb_event);
  void MousePressed(SDL::MouseButtonEvent mouse_button_event);
  void MouseReleased(SDL::MouseButtonEvent mouse_button_event);
  void MouseMoved(SDL::MouseMotionEvent mouse_motion_event);
  void MouseWheelMoved(SDL::MouseWheelEvent mouse_wheel_event);

  double* GetCameraSpeedPtr();
  double* GetCameraRotationSpeedPtr();

 private:
  void AdvanceLookDirViaKeyboard(util::Timer::SecondsUnit elapsed);
  void AdvancePositionForward(util::Timer::SecondsUnit elapsed);
  void AdvancePositionVertical(util::Timer::SecondsUnit elapsed);
  void AdvancePositionHorizontal(util::Timer::SecondsUnit elapsed);

  int ScancodesSumTotal(std::initializer_list<SDL::Scancode> positive,
                        std::initializer_list<SDL::Scancode> negative);

 private:
  Camera* camera_;
  std::unique_ptr<std::array<bool, Impl::kAmountOfScancodes>> held_;

  double camera_speed_ = 4.0;
  double camera_rotation_speed_ = 0.7;
};

}  // namespace renderer
