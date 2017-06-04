#pragma once

#include "Contract.h"

#include <mutex>
#include <numeric>
#include <vector>

#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <iomanip>

constexpr auto workaround = 0;

class HitQueue
{
public:
	explicit HitQueue(int numTimeSlots)
		: hitsPerTimeSlot_(numTimeSlots + workaround)
		, activeHitsSum_(0)
		, windowBegin_(0)
		, windowEnd_(numTimeSlots + workaround - 1)
		, log_(logFileName().c_str())
		, detailedLog_((logFileName() + "detailed").c_str())
	{
	}

	~HitQueue()
	{
		const auto sum = CalculateSum(windowBegin_, windowEnd_);
		std::cout << "Moved " << movedCount << " times\n"
		    << "startSlot_ = " << windowBegin_
			<< ";\n\tcurrentSlot_ = " << windowEnd_
			<< ";\n\tCalculateSum(startSlot_, currentSlot_) = " << sum
			<< ";\n\tactiveHitsSum_ = " << activeHitsSum_
			<< ";\n\thitsPerTimeSlot_.size() = " << hitsPerTimeSlot_.size()
			<< ";\n\thitsPerTimeSlot_.at(startSlot_) = " << hitsPerTimeSlot_.at(windowBegin_)
			<< ";\n\tDELTA = " << sum - activeHitsSum_ << '\n';
	}

	void NextTimeSlot()
	{
		std::lock_guard<std::mutex> l(mutex_);

		activeHitsSum_ -= hitsPerTimeSlot_.at(windowBegin_);
		MoveNext();

		CheckContract();
	}

	void CheckContract()
	{
		const auto sum = CalculateSum(windowBegin_, windowEnd_);
		if (sum != activeHitsSum_)
		{
			log_ << "CHECK FOR SUM FAILED:\n\tstartSlot_ = " << windowBegin_
				<< ";\n\tcurrentSlot_ = " << windowEnd_
				<< ";\n\tCalculateSum(startSlot_, currentSlot_) = " << sum
				<< ";\n\tactiveHitsSum_ = " << activeHitsSum_
				<< ";\n\thitsPerTimeSlot_.size() = " << hitsPerTimeSlot_.size()
				<< ";\n\thitsPerTimeSlot_.at(startSlot_) = " << hitsPerTimeSlot_.at(windowBegin_)
				<< ";\n\tDELTA = " << sum - activeHitsSum_ << '\n';
			for (int index = windowBegin_; index != windowEnd_; index = NextIndex(index))
			{
				detailedLog_
					<< std::setw(10) << index
					<< std::setw(30) << "queue[index]=" << hitsPerTimeSlot_.at(index)
					<< '\n';
			}
			detailedLog_
				<< std::setw(10) << windowEnd_
				<< std::setw(30) << "queue[index]=" << hitsPerTimeSlot_.at(windowEnd_)
				<< '\n';
		}
		CONTRACT_EXPECT(CalculateSum(windowBegin_, windowEnd_) == activeHitsSum_);
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
	static std::string logFileName()
	{
		return std::string("RateLimiter") +
			std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())
			+ ".log";
	}

	int movedCount = 0;

	void MoveNext()
	{
		movedCount++;
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

	int CalculateSum(int startIndex, int currentIndex) const
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
	std::ofstream log_;
	std::ofstream detailedLog_;
};
