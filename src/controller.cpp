#include "controller.hpp"

#include <algorithm>
#include <glog/logging.h>
#include <Eigen/Geometry>
#include "util/util.hpp"

namespace renderer {

namespace {

const Vector3d kXAxis = Vector3d::UnitX();
const Vector3d kYAxis = Vector3d::UnitY();
const Vector3d kNegZAxis = -Vector3d::UnitZ();

}  // namespace

Controller::Controller(Camera& camera)
    : camera_(&camera), held_(std::make_unique<std::array<bool, Impl::kAmountOfScancodes>>()) {
  std::fill(held_->begin(), held_->end(), false);
}

void Controller::Advance(util::Timer::SecondsUnit elapsed) {
  AdvancePositionForward(elapsed);
  AdvancePositionHorizontal(elapsed);
  AdvancePositionVertical(elapsed);
  AdvanceLookDirViaKeyboard(elapsed);
}

void Controller::KeyPressed(SDL::KeyboardEvent kb_event) {
  (*held_)[kb_event.scancode] = true;
}

void Controller::KeyReleased(SDL::KeyboardEvent kb_event) {
  (*held_)[kb_event.scancode] = false;
}

void Controller::MousePressed(SDL::MouseButtonEvent mouse_button_event) {
}

void Controller::MouseReleased(SDL::MouseButtonEvent mouse_button_event) {
}

void Controller::MouseMoved(SDL::MouseMotionEvent mouse_motion_event) {
}

void Controller::MouseWheelMoved(SDL::MouseWheelEvent mouse_wheel_event) {
}

double* Controller::GetCameraSpeedPtr() {
  return &camera_speed_;
}

double* Controller::GetCameraRotationSpeedPtr() {
  return &camera_rotation_speed_;
}

void Controller::AdvanceLookDirViaKeyboard(util::Timer::SecondsUnit elapsed) {
  const double yaw = ScancodesSumTotal({SDL_SCANCODE_LEFT}, {SDL_SCANCODE_RIGHT}) *
                     camera_rotation_speed_ * elapsed;
  const double pitch =
      ScancodesSumTotal({SDL_SCANCODE_UP}, {SDL_SCANCODE_DOWN}) * camera_rotation_speed_ * elapsed;
  const double roll =
      ScancodesSumTotal({SDL_SCANCODE_E}, {SDL_SCANCODE_Q}) * camera_rotation_speed_ * elapsed;
  auto rotation_transform = Eigen::AngleAxisd(roll, kNegZAxis) * Eigen::AngleAxisd(pitch, kXAxis) *
                            Eigen::AngleAxisd(yaw, kYAxis);

  Quaternion rot_quat(rotation_transform);
  camera_->RotateLookDir(rot_quat);
}

void Controller::AdvancePositionForward(util::Timer::SecondsUnit elapsed) {
  const Vector3d forward_offset = camera_->GetForwardDirection() * camera_speed_;
  int forward = ScancodesSumTotal({SDL_SCANCODE_W}, {SDL_SCANCODE_S});
  camera_->Offset(forward_offset * (forward * elapsed));
}

void Controller::AdvancePositionVertical(util::Timer::SecondsUnit elapsed) {
  const Vector3d vert_offset = camera_->GetUpDirection() * camera_speed_;
  int up = ScancodesSumTotal({SDL_SCANCODE_SPACE}, {SDL_SCANCODE_Z});
  camera_->Offset(vert_offset * (up * elapsed));
}

void Controller::AdvancePositionHorizontal(util::Timer::SecondsUnit elapsed) {
  const Vector3d hor_offset = camera_->GetRightDirection() * camera_speed_;
  int right = ScancodesSumTotal({SDL_SCANCODE_D}, {SDL_SCANCODE_A});
  camera_->Offset(hor_offset * (right * elapsed));
}

int Controller::ScancodesSumTotal(std::initializer_list<SDL::Scancode> positive,
                                  std::initializer_list<SDL::Scancode> negative) {
  bool any_positive = false;
  bool any_negative = false;
  for (SDL::Scancode scancode : positive) {
    any_positive |= (*held_)[scancode];
  }

  for (SDL::Scancode scancode : negative) {
    any_negative |= (*held_)[scancode];
  }

  int result = any_positive - any_negative;
  return result;
}

}  // namespace renderer
