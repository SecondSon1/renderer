#include "scene/lighting.hpp"

#include <utility>
#include "util/util.hpp"

namespace renderer {

LightSourceBase LightSource::GetBase() const {
  LightSourceBase result;
  auto handlers = util::overloaded{
      [&result](AmbientLight ambient) { result = ambient; },
      [&result](DirectionalLight directional) { result = directional; },
      [&result](PointLightSource point_light_source) { result = point_light_source; },
  };
  std::visit(std::move(handlers), *this);
  return result;
}

}  // namespace renderer
