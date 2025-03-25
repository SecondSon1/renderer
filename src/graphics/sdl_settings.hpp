#pragma once

#include <cstdint>

namespace SDL {

namespace settings {

namespace pixel_fmt {

constexpr size_t kPixelSizeInBytes = 3;

constexpr uint32_t kROffsetInBytes = 0;
constexpr uint32_t kGOffsetInBytes = 1;
constexpr uint32_t kBOffsetInBytes = 2;

constexpr uint32_t kRShift = (kPixelSizeInBytes - kROffsetInBytes - 1) * 8;
constexpr uint32_t kGShift = (kPixelSizeInBytes - kGOffsetInBytes - 1) * 8;
constexpr uint32_t kBShift = (kPixelSizeInBytes - kBOffsetInBytes - 1) * 8;

static constexpr uint32_t kRMask = ((1u << 8) - 1) << kRShift;
static constexpr uint32_t kGMask = ((1u << 8) - 1) << kGShift;
static constexpr uint32_t kBMask = ((1u << 8) - 1) << kBShift;

}  // namespace PixelFormat

}  // namespace Settings

}  // namespace SDL
