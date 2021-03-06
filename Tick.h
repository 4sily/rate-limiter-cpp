#pragma once

#include <chrono>
#include <functional>
#include <thread>
#include <atomic>

using Clock = std::chrono::steady_clock;

class Tick
{
public:
    Tick(std::chrono::microseconds timeBetweenTicks,
         std::function<void()>&& onTick)
        : timeBetweenTicks_(timeBetweenTicks)
        , onTick_(std::move(onTick))
        , active_(true)
        , timerThread_([this] { Loop(); }) // note initialization order is very important here; 
                                            // thread should start last
    {}

    void Deactivate() { active_ = false; timerThread_.join(); }

private:
    void Loop() const
    {
        while (active_)
        {
            for (auto start = Clock::now(), now = start;
                 now < start + timeBetweenTicks_;
                 now = Clock::now()) { /* Until next tick */
            }
            onTick_();	// may take significant time!
        }
    }

    const std::function<void()> onTick_;
    const std::chrono::microseconds timeBetweenTicks_;
    std::atomic<bool> active_;
    std::thread timerThread_;
};
