#include <glog/logging.h>
#include "fs/root_finder.hpp"
#include "app.hpp"
#include "except.hpp"

namespace {

void InitializeGlobalObjects(const char *argv0) {
  google::InitGoogleLogging(argv0);
}

}  // namespace

int main(int argc, char *argv[]) {
  InitializeGlobalObjects(argv[0]);

  renderer::MoveToRoot();

  try {
    renderer::Application app;
    app.Run();
  } catch (...) {
    except::react();
  }

  return 0;
}
