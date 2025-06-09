#include "fs/datalist_parser.hpp"

#include <cctype>
#include <cassert>

namespace renderer {

DatalistParser::DatalistParser(std::istream& stream) : stream_(&stream) {
}

std::optional<DatalistParser::Entry> DatalistParser::GetNext() {
  std::string result;
  if (!std::getline(*stream_, result) || result.empty()) {
    return std::nullopt;
  }
  size_t leftmost = 0;
  size_t rightmost = result.size() - 1;
  while (leftmost < rightmost && std::isspace(result[leftmost])) {
    ++leftmost;
  }
  while (leftmost < rightmost && std::isspace(result[rightmost])) {
    --rightmost;
  }
  if (std::isspace(result[leftmost])) {
    return std::nullopt;
  }

  result = result.substr(leftmost, rightmost - leftmost + 1);
  assert(!result.empty());
  if (result[0] == '#') {
    return GetNext();
  }

  return result;
}

}  // namespace renderer
