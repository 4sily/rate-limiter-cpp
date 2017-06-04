#pragma once

#include "Contract.h"

#include <mutex>
#include <numeric>
#include <vector>

constexpr auto workaround = 0;

class HitQueue
{
public:
    explicit HitQueue(int numTimeFrames)
        : hitsPerTimeFrame_(numTimeFrames + workaround)
        , activeHitsSum_(0)
        , windowBegin_(0)
        , windowEnd_(numTimeFrames + workaround - 1)
    {
    }

    void NextTimeFrame()
    {
        std::lock_guard<std::mutex> l(mutex_);

        activeHitsSum_ -= hitsPerTimeFrame_.at(windowBegin_);
        MoveNext();

        CONTRACT_EXPECT(sumOfRange(windowBegin_, windowEnd_) == activeHitsSum_);
    }

    void AddHit()
    {
        std::lock_guard<std::mutex> l(mutex_);
        hitsPerTimeFrame_.at(windowEnd_)++;
        activeHitsSum_++;
    }

    int ActiveSum() const
    {
        std::lock_guard<std::mutex> l(mutex_);
        return activeHitsSum_;
    }

private:

    void MoveNext()
    {
        windowBegin_ = NextIndex(windowBegin_);
        windowEnd_ = NextIndex(windowEnd_);
        hitsPerTimeFrame_.at(windowEnd_) = 0;
    }

    int NextIndex(int currentIndex) const
    {
        // circular iteration
        return currentIndex < static_cast<int>(hitsPerTimeFrame_.size()) - 1
            ? currentIndex + 1
            : 0;
    }

    // Sum of range in the circular buffer.
    int sumOfRange(int startIndex, int currentIndex) const
    {
        const auto b = hitsPerTimeFrame_.begin();
        return startIndex < currentIndex
            ? std::accumulate(b + startIndex,
                              b + currentIndex + 1,
                              0)
            : std::accumulate(b + startIndex,
                              hitsPerTimeFrame_.end(),
                              0) + std::accumulate(b, b + currentIndex + 1, 0);
    }

    std::vector<int> hitsPerTimeFrame_;
    mutable std::mutex mutex_;
    int activeHitsSum_;
    int windowBegin_;
    int windowEnd_;
};
