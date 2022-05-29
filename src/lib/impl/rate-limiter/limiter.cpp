#include <chrono>
#include <mutex>

#include <rate-limiter/limiter.h>

namespace rate_limiter {

Limiter::Limiter(std::uint64_t maxRPS, std::uint64_t timeFramesPS)
    : hitQueue_(timeFramesPS)
      // note that callbacks may start coming right after this
      ,
      tick_(std::chrono::seconds(1) / timeFramesPS,
            [this] { OnTimeFrameBoundary(); }),
      maxRPS_(maxRPS) {}

Limiter::~Limiter() {
  tick_.Deactivate(); // lifecycle issues are still possible, see Tick::Loop()
}

HttpResult::Code Limiter::ValidateRequest() {
  std::lock_guard<std::mutex> l(mutex_);
  if (hitQueue_.ActiveSum() >= maxRPS_) {
    return HttpResult::Code::TooManyRequests;
  }
  hitQueue_.AddHit();
  return HttpResult::Code::Ok;
}

std::uint64_t Limiter::MaxRPS() const { return maxRPS_; }

void Limiter::OnTimeFrameBoundary() {
  std::lock_guard<std::mutex> l(mutex_);
  hitQueue_.NextTimeFrame();
}

} // namespace rate_limiter