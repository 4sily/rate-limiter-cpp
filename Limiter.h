#pragma once

#include "Tick.h"

#include <chrono>
#include <list>

struct HttpResult
{
	enum class Code {
		Ok = 200,
		TooManyRequests = 429
	};

	HttpResult(Code code) : code(code)
	{}
	
	const Code code;
};

class Limiter
{
public:
	Limiter(int maxRequestCount, std::chrono::milliseconds slotDuration)
		: tick_(slotDuration, [this] { OnTimeSlotBoundary(); })	// note that callbacks may start coming right after this
		, requestsPerTimeSlot_({0})
		, activeRequestCount_(0)
		, maxRequestCount_(maxRequestCount)
	{
	}

	~Limiter()
	{
		tick_.Deactivate();	// lifecycle issues are still possible, see Tick::Loop()
	}

	HttpResult::Code ValidateRequest()
	{
		if (activeRequestCount_ >= maxRequestCount_)
		{
			return HttpResult::Code::TooManyRequests;
		}
		activeRequestCount_++;
		requestsPerTimeSlot_.back()++;
		return HttpResult::Code::Ok;
	}

private:
	void OnTimeSlotBoundary()
	{
		requestsPerTimeSlot_.push_back(0);
		activeRequestCount_ -= requestsPerTimeSlot_.front();
		requestsPerTimeSlot_.pop_front();
	}

	Tick tick_;
	std::list<int> requestsPerTimeSlot_; // bad idea; would be better to use circular queue
	int activeRequestCount_;
	const int maxRequestCount_;
};
