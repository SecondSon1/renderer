#include "fs/datalist_parser.hpp"

namespace renderer {

DatalistParser::DatalistParser(std::istream& stream) : stream_(&stream) {
}

std::optional<DatalistParser::Entry> DatalistParser::GetNext() {
  std::string result;
  if (!std::getline(*stream_, result)) {
    return std::nullopt;
  }
  return result;
}

}  // namespace renderer
