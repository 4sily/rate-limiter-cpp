#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

#include <rate-limiter/hit_queue.h>

namespace rate_limiter {

HitQueue::HitQueue(std::uint64_t numTimeFrames)
    : hitsPerTimeFrame_(numTimeFrames), activeHitsSum_(0), windowBegin_(0),
      windowEnd_(numTimeFrames - 1) {}

void HitQueue::NextTimeFrame() {
  activeHitsSum_ -= hitsPerTimeFrame_.at(windowBegin_);
  MoveNext();

  CONTRACT_EXPECT(SumOfRange(windowBegin_, windowEnd_) == activeHitsSum_);
}

void HitQueue::AddHit() {
  hitsPerTimeFrame_.at(windowEnd_)++;
  activeHitsSum_++;
}

std::uint64_t HitQueue::ActiveSum() const { return activeHitsSum_; }

void HitQueue::MoveNext() {
  windowBegin_ = NextIndex(windowBegin_);
  windowEnd_ = NextIndex(windowEnd_);
  hitsPerTimeFrame_.at(windowEnd_) = 0;
}

std::uint64_t HitQueue::NextIndex(std::uint64_t currentIndex) const {
  // circular iteration
  return currentIndex < static_cast<std::uint64_t>(hitsPerTimeFrame_.size()) - 1
             ? currentIndex + 1
             : 0;
}

// Sum of range in the circular buffer.
std::uint64_t HitQueue::SumOfRange(std::uint64_t startIndex,
                                   std::uint64_t currentIndex) const {
  const auto b = hitsPerTimeFrame_.begin();
  return startIndex < currentIndex
             ? std::accumulate(b + startIndex, b + currentIndex + 1, 0)
             : std::accumulate(b + startIndex, hitsPerTimeFrame_.end(), 0) +
                   std::accumulate(b, b + currentIndex + 1, 0);
}

} // namespace rate_limiter