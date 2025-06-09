#include <util/timer.hpp>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <thread>

namespace chrono = std::chrono;

namespace renderer {

namespace util {

Timer::Timer() noexcept {
  Reset();
}

void Timer::Reset() noexcept {
  begin_ = Clock::now();
  lap_begin_ = begin_;
}

using SecondsUnit = Timer::SecondsUnit;

static constexpr SecondsUnit kUsecToSecMultiplier = 1 / 1000000.f;

SecondsUnit Timer::SecondsSinceStart() const noexcept {
  return SecondsSince(Clock::now(), begin_);
}

SecondsUnit Timer::SecondsIntoLap() const noexcept {
  return SecondsSince(Clock::now(), lap_begin_);
}

SecondsUnit Timer::Lap() noexcept {
  auto now = Clock::now();
  SecondsUnit result = SecondsSince(now, lap_begin_);
  lap_begin_ = now;
  return result;
}

namespace {

constexpr SecondsUnit kSecondsToMillisecondsMultiplier = 1000.f;

}

void Timer::WaitUntilLapIs(SecondsUnit seconds) const noexcept {
  auto now = Clock::now();
  if (SecondsSince(now, lap_begin_) >= seconds) {
    return;
  }
  uint32_t millis_count = std::round(seconds * kSecondsToMillisecondsMultiplier);
  chrono::milliseconds millis(millis_count);
  std::this_thread::sleep_until(now + millis);
}

float Timer::SecondsSince(TimePoint now, TimePoint since_here) noexcept {
  auto usec_elapsed = chrono::duration_cast<chrono::microseconds>(now - since_here).count();
  return usec_elapsed * kUsecToSecMultiplier;
}

}  // namespace util

}  // namespace renderer
