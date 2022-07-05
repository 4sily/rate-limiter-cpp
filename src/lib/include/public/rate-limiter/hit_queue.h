#ifndef F756F479_B11E_4C69_A24F_49F3CB19C768
#define F756F479_B11E_4C69_A24F_49F3CB19C768

#pragma once

#include <atomic>
#include <cstdint>
#include <numeric>
#include <vector>

#include <rate-limiter/contract.h>

namespace rate_limiter {

class HitQueue {
public:
  explicit HitQueue(std::uint64_t numTimeFrames);

  void NextTimeFrame();
  void AddHit();
  std::uint64_t ActiveSum() const;

private:
  void MoveNext();

  std::uint64_t NextIndex(std::uint64_t currentIndex) const;

  // Sum of range in the circular buffer.
  std::uint64_t SumOfRange(std::uint64_t startIndex,
                           std::uint64_t currentIndex) const;

private:
  std::vector<std::uint64_t> hitsPerTimeFrame_;
  std::atomic<std::uint64_t> activeHitsSum_;
  std::atomic<std::uint64_t> windowBegin_;
  std::atomic<std::uint64_t> windowEnd_;
};

} // namespace rate_limiter

#endif /* F756F479_B11E_4C69_A24F_49F3CB19C768 */
