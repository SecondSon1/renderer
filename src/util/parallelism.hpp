#pragma once

#include <memory>
#include <execution>
#include <omp.h>
#include <glog/logging.h>

namespace renderer {

namespace util {

template <typename T>
std::unique_ptr<T[]> FillParallel(size_t sz, T elem) {
  std::unique_ptr<T[]> result = std::make_unique_for_overwrite<T[]>(sz);
  T* begin = &result[0];
  std::fill(std::execution::par, begin, begin + sz, elem);
  return result;
}

}  // namespace util

}  // namespace renderer
