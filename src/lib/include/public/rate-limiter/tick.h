#ifndef E7F6C6D9_84A9_4B1D_9B0E_97403BF8C0E4
#define E7F6C6D9_84A9_4B1D_9B0E_97403BF8C0E4

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

namespace rate_limiter {

using Clock = std::chrono::steady_clock;

class Tick {
public:
  Tick(std::chrono::microseconds timeBetweenTicks,
       std::function<void()> &&onTick);

  void Deactivate();

private:
  void Loop() const;

private:
  const std::function<void()> onTick_;
  const std::chrono::microseconds timeBetweenTicks_;
  std::atomic<bool> active_;
  std::thread timerThread_;
};

} // namespace rate_limiter

#endif /* E7F6C6D9_84A9_4B1D_9B0E_97403BF8C0E4 */
