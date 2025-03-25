#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include "util/timer.hpp"

namespace renderer {

namespace util {

namespace {

constexpr uint32_t kQueueSize = 64;

}

class TimingsMeasurer {
 public:
  using SecondsUnit = Timer::SecondsUnit;

 public:
  TimingsMeasurer();

  void AddTiming(SecondsUnit timing);

  SecondsUnit GetAverageTiming() const;

private:
  void RecomputeSum();
  void RecomputeAvg();

 private:
  std::unique_ptr<std::array<SecondsUnit, kQueueSize>> timings_;
  SecondsUnit sum_ = 0;
  uint32_t idx_ = 0;
  SecondsUnit avg_timing_ = 0;
  SecondsUnit since_last_upd_ = 0;
};

}  // namespace util

}  // namespace renderer
