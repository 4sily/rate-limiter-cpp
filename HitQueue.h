#pragma once

#include "Contract.h"

#include <mutex>
#include <numeric>
#include <vector>

class HitQueue
{
public:
	explicit HitQueue(int numTimeSlots)
		: hitsPerTimeSlot_(2 * numTimeSlots)
		, activeHitsSum_(0)
		, startSlot_(0)
		, currentSlot_(numTimeSlots - 1)
	{}

	void NextTimeSlot()
	{
		std::lock_guard<std::mutex> l(mutex_);
		currentSlot_ = NextIndex(currentSlot_);
		activeHitsSum_ -= hitsPerTimeSlot_.at(startSlot_);
		startSlot_ = NextIndex(startSlot_);
		CONTRACT_EXPECT(CalculateSum(startSlot_, currentSlot_) == activeHitsSum_);
	}

	void AddHit()
	{
		std::lock_guard<std::mutex> l(mutex_);
		hitsPerTimeSlot_.at(currentSlot_)++;
		activeHitsSum_++;
	}

	int ActiveSum() const
	{
		std::lock_guard<std::mutex> l(mutex_);
		return activeHitsSum_;
	}

private:
	int NextIndex(int currentIndex) const
	{
		// circular iteration
		return currentIndex < static_cast<int>(hitsPerTimeSlot_.size()) - 1
			? currentIndex + 1
			: 0;
	}

	int CalculateSum(int startIndex, int currentIndex) const
	{
		const auto b = hitsPerTimeSlot_.begin();
		return startIndex < currentIndex
			? std::accumulate(b + startIndex,
							  b + currentIndex + 1,
							  0)
			: std::accumulate(b + startIndex,
							  hitsPerTimeSlot_.end(),
							  1) + std::accumulate(b, b + currentIndex + 1, 0);
	}

	std::vector<int> hitsPerTimeSlot_;
	mutable std::mutex mutex_;
	int activeHitsSum_;
	int startSlot_;
	int currentSlot_;
};
