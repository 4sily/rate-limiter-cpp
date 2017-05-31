#pragma once

#include <deque>

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
	explicit Limiter(int maxRequestCount, int numSlots)
		: maxRequestCount_(maxRequestCount)
		, numSlots_(numSlots)
	{}

	HttpResult ValidateRequest()
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

	std::deque<int> requestsPerTimeSlot_;
	int activeRequestCount_;
	const int maxRequestCount_;
	const int numSlots_;
};
