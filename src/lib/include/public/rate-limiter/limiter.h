#ifndef DFE34E7B_1741_4AF8_835F_161619EBFEBF
#define DFE34E7B_1741_4AF8_835F_161619EBFEBF

#pragma once

#include <chrono>
#include <mutex>

#include <rate-limiter/hit_queue.h>
#include <rate-limiter/http_result.h>
#include <rate-limiter/tick.h>

namespace rate_limiter {

class Limiter {
public:
  Limiter(std::uint64_t maxRPS, std::uint64_t timeFramesPerSecond);

  ~Limiter();

  HttpResult::Code ValidateRequest();

  std::uint64_t MaxRPS() const;

private:
  void OnTimeFrameBoundary();

private:
  std::mutex mutex_;
  HitQueue hitQueue_;
  Tick tick_;
  const std::uint64_t maxRPS_;
};
} // namespace rate_limiter

#endif /* DFE34E7B_1741_4AF8_835F_161619EBFEBF */
