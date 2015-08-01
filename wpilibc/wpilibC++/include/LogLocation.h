#pragma once
#include <string>
#include <sstream>

struct LogLocation {
  std::string file;
  std::string func;
  int line;
  operator std::string() const;
  operator const char *() const { return (operator std::string()).c_str(); }
  // Shortens filename so that it doesn't have all the useless
  // /home/$USER/path/to/src stuff on the front.
  static std::string ShortFilename(std::string name);
};
