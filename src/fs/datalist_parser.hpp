#pragma once

#include <istream>
#include <optional>
#include <string>

namespace renderer {

class DatalistParser {
 public:
  using Entry = std::string;
 public:
  DatalistParser(std::istream& stream);

  std::optional<Entry> GetNext();

 private:
  std::istream* stream_;
};

}  // namespace renderer
