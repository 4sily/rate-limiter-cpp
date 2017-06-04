#pragma once

#include <chrono>
#include <functional>
#include <thread>
#include <atomic>

class Tick
{
public:
	Tick(std::chrono::microseconds timeBetweenTicks,
		 std::function<void()>&& onTick)
		: timeBetweenTicks_(timeBetweenTicks)
		, onTick_(onTick)
		, active_(true)
		, workerThread_([this] { Loop(); })	// note initialization order is very important here; 
											// thread should start last
	{}

	void Deactivate() { active_ = false; workerThread_.join(); }

	bool IsActive() const { return active_; }

private:
	void Loop()
	{
		while (active_)
		{
			for (auto start = std::chrono::steady_clock::now(), now = start;
				 now < start + timeBetweenTicks_;
				 now = std::chrono::high_resolution_clock::now());
			onTick_();	// may take significant time!
		}
	}

	std::function<void()> onTick_;
	std::chrono::milliseconds timeBetweenTicks_;
	std::atomic<bool> active_;
	std::thread workerThread_;
};
