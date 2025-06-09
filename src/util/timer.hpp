#pragma once

#include <chrono>

namespace renderer {

namespace util {

class Timer {
 public:
  using Clock = std::chrono::high_resolution_clock;
  using TimePoint = Clock::time_point;
  using SecondsUnit = float;

 public:
  Timer() noexcept;

  void Reset() noexcept;

  SecondsUnit SecondsSinceStart() const noexcept;
  SecondsUnit SecondsIntoLap() const noexcept;
  SecondsUnit Lap() noexcept;

  void WaitUntilLapIs(SecondsUnit seconds) const noexcept;

 private:
  static SecondsUnit SecondsSince(TimePoint now, TimePoint since_here) noexcept;

 private:
  TimePoint begin_;
  TimePoint lap_begin_;
};

}  // namespace util

}  // namespace renderer
