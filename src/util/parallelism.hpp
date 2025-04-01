#pragma once

#include <memory>
#include <execution>
#include <glog/logging.h>

namespace renderer {

namespace util {

template <typename T>
std::unique_ptr<T[]> FillParallel(size_t sz, T elem) {
  std::unique_ptr<T[]> result = std::make_unique_for_overwrite<T[]>(sz);
  T* begin = &result[0];
#if defined(__clang__) && defined(__apple_build_version__)
  // Apple Clang
  std::fill(begin, begin + sz, elem);
#else
  // Not Apple Clang
  std::fill(std::execution::par_unseq, begin, begin + sz, elem);
#endif
  return result;
}

}  // namespace util

}  // namespace renderer
