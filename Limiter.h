#pragma once

#include "HitQueue.h"
#include "Tick.h"

#include <chrono>
#include <mutex>

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
    Limiter(int maxRPS, int timeFramesPS)
        : hitQueue_(timeFramesPS)
        // note that callbacks may start coming right after this
        , tick_(std::chrono::seconds(1) / timeFramesPS, [this] { OnTimeFrameBoundary(); })
        , maxRPS_(maxRPS)
    {
    }

    ~Limiter()
    {
        tick_.Deactivate();	// lifecycle issues are still possible, see Tick::Loop()
    }

    HttpResult::Code ValidateRequest()
    {
        std::lock_guard<std::mutex> l(mutex_);
        if (hitQueue_.ActiveSum() >= maxRPS_)
        {
            return HttpResult::Code::TooManyRequests;
        }
        hitQueue_.AddHit();
        return HttpResult::Code::Ok;
    }

    int maxRPS() const { return maxRPS_; }

private:
    void OnTimeFrameBoundary()
    {
        std::lock_guard<std::mutex> l(mutex_);
        hitQueue_.NextTimeFrame();
    }

    std::mutex mutex_;
    HitQueue hitQueue_;
    Tick tick_;
    const int maxRPS_;
};
