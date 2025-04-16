#include "except.hpp"

#include <glog/logging.h>

namespace except {

void react() {
  try {
    throw;
  } catch (std::exception& e) {
    LOG(FATAL) << "Exception thrown: " << e.what();
  } catch (...) {
    LOG(FATAL) << "Unknown object thrown";
  }
}

}  // namespace except
