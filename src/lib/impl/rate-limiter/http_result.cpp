#include <iostream>

#include <rate-limiter/http_result.h>

namespace rate_limiter {

HttpResult::HttpResult(Code code) : code(code) {}

std::ostream &operator<<(std::ostream &os, HttpResult::Code code) {
  const auto &str = code == HttpResult::Code::Ok ? "Ok (200)"
                    : code == HttpResult::Code::TooManyRequests
                        ? "Too many requests (429)"
                        : "Unknown code";
  os << str;
  return os;
}

} // namespace rate_limiter
