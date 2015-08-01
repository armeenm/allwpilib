#include "LogLocation.h"

LogLocation::operator std::string() const {
  std::stringstream formatted;
  // TODO: Consider having separate formattings for making it easily parseable
  // (eg, file:func:line) versus pretty to read (eg, file at line in func).
  formatted << file << ":" << func << ":" << line;
  return formatted.str();
}

std::string LogLocation::ShortFilename(std::string name) {
  // TODO: Abstract this out.
  auto start = name.find("wpilib");
  if (start == std::string::npos) {
    start = name.rfind("src");
    if (start == std::string::npos) return name;
  }
  return std::string(name.begin() + start, name.end());
}
