#pragma once

#include "Contract.h"

#include <mutex>
#include <numeric>
#include <vector>

constexpr auto workaround = 0;

class HitQueue
{
public:
	explicit HitQueue(int numTimeSlots)
		: hitsPerTimeSlot_(numTimeSlots + workaround)
		, activeHitsSum_(0)
		, windowBegin_(0)
		, windowEnd_(numTimeSlots + workaround - 1)
	{
	}

	void NextTimeSlot()
	{
		std::lock_guard<std::mutex> l(mutex_);

		activeHitsSum_ -= hitsPerTimeSlot_.at(windowBegin_);
		MoveNext();

		CONTRACT_EXPECT(sumOfRange(windowBegin_, windowEnd_) == activeHitsSum_);
	}

	void AddHit()
	{
		std::lock_guard<std::mutex> l(mutex_);
		hitsPerTimeSlot_.at(windowEnd_)++;
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
		hitsPerTimeSlot_.at(windowEnd_) = 0;
	}

	int NextIndex(int currentIndex) const
	{
		// circular iteration
		return currentIndex < static_cast<int>(hitsPerTimeSlot_.size()) - 1
			? currentIndex + 1
			: 0;
	}

	// Sum of range in the circular buffer.
	int sumOfRange(int startIndex, int currentIndex) const
	{
		const auto b = hitsPerTimeSlot_.begin();
		return startIndex < currentIndex
			? std::accumulate(b + startIndex,
							  b + currentIndex + 1,
							  0)
			: std::accumulate(b + startIndex,
							  hitsPerTimeSlot_.end(),
							  0) + std::accumulate(b, b + currentIndex + 1, 0);
	}

	std::vector<int> hitsPerTimeSlot_;
	mutable std::mutex mutex_;
	int activeHitsSum_;
	int windowBegin_;
	int windowEnd_;
};
