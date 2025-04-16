#include "util/timings_measurer.hpp"

#include <algorithm>

namespace renderer {

namespace util {

using SecondsUnit = TimingsMeasurer::SecondsUnit;

namespace {

constexpr uint32_t kRecomputeSumPer = 256;
constexpr SecondsUnit kRecomputeAvgPerSeconds = 0.5;

}  // namespace

TimingsMeasurer::TimingsMeasurer()
    : timings_(std::make_unique<std::array<SecondsUnit, kQueueSize>>()) {
  std::fill(timings_->begin(), timings_->end(), 0);
}

void TimingsMeasurer::AddTiming(SecondsUnit timing) {
  auto& slot = (*timings_)[idx_++ % kQueueSize];
  sum_ -= slot;
  slot = timing;
  sum_ += timing;
  if (idx_ % kRecomputeSumPer == 0) {
    RecomputeSum();
  }

  since_last_upd_ += timing;
  if (since_last_upd_ > kRecomputeAvgPerSeconds) {
    RecomputeAvg();
  }
}

SecondsUnit TimingsMeasurer::GetAverageTiming() const {
  return avg_timing_;
}

void TimingsMeasurer::RecomputeSum() {
  sum_ = 0;
  for (auto x : *timings_) {
    sum_ += x;
  }
}

void TimingsMeasurer::RecomputeAvg() {
  since_last_upd_ = 0;
  uint32_t count = std::min(idx_, kQueueSize);
  avg_timing_ = sum_ / count;
}

}  // namespace util

}  // namespace renderer
