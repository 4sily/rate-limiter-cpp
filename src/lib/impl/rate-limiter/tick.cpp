#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

#include <rate-limiter/tick.h>

namespace rate_limiter {

Tick::Tick(std::chrono::microseconds timeBetweenTicks,
           std::function<void()> &&onTick)
    : timeBetweenTicks_(timeBetweenTicks), onTick_(std::move(onTick)),
      active_(true), timerThread_([this] {
        Loop();
      }) // note initialization order is very important here;
         // thread should start last
{}

void Tick::Deactivate() {
  active_ = false;
  timerThread_.join();
}

void Tick::Loop() const {
  while (active_) {
    for (auto start = Clock::now(), now = start;
         now < start + timeBetweenTicks_;
         now = Clock::now()) { /* Until next tick */
    }
    onTick_(); // may take significant time!
  }
}

} // namespace rate_limiter
