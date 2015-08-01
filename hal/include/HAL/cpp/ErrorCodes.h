#pragma once
#include <map>

class ErrorCodes {
 public:
  static ErrorCodes &GetInstance();
  const char *Get(int code);
  int GetCode(const char *name);
  bool Add(int code, const char *msg, const char *name=nullptr);

 private:
  ErrorCodes();
  std::map<int, const char *> m_codes;
  std::map<const char *, int> m_names;
};

