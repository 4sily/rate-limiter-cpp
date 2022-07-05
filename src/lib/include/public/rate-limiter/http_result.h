#ifndef B4FFE348_8D15_408B_825B_A0E82BB18EE0
#define B4FFE348_8D15_408B_825B_A0E82BB18EE0

#pragma once

#include <iostream>

namespace rate_limiter {

struct HttpResult {
  enum class Code : int { Ok = 200, TooManyRequests = 429 };

  HttpResult(Code code);

  const Code code;
};

std::ostream &operator<<(std::ostream &os, HttpResult::Code code);

} // namespace rate_limiter

#endif /* B4FFE348_8D15_408B_825B_A0E82BB18EE0 */
