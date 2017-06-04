#pragma once

#include "HitQueue.h"
#include "Tick.h"

#include <chrono>

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
    Limiter(int maxRPS, int timeSlotsPS)
        : hitQueue_(timeSlotsPS)
        // note that callbacks may start coming right after this
        , tick_(std::chrono::seconds(1) / timeSlotsPS, [this] { OnTimeSlotBoundary(); })
        , maxRPS_(maxRPS)
    {
    }

    ~Limiter()
    {
        tick_.Deactivate();	// lifecycle issues are still possible, see Tick::Loop()
    }

    HttpResult::Code ValidateRequest()
    {
        if (hitQueue_.ActiveSum() >= maxRPS_)
        {
            return HttpResult::Code::TooManyRequests;
        }
        hitQueue_.AddHit();
        return HttpResult::Code::Ok;
    }

    int maxRPS() const { return maxRPS_; }

private:
    void OnTimeSlotBoundary()
    {
        hitQueue_.NextTimeSlot();
    }

    HitQueue hitQueue_;
    Tick tick_;
    const int maxRPS_;
};
