#include "Limiter.h"
#include "Testing.h"

#include <iostream>

auto TestWithPeakLoadInTheBeginning(int maxAllowedRps)
{
	const auto startTime = std::chrono::steady_clock::now();
	int sentRequestsCount = 0;

	Limiter limiter(maxAllowedRps, 10000);
	for (; sentRequestsCount < maxAllowedRps; ++sentRequestsCount)
	{
		ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
	}
	auto expectedEndResult = std::chrono::steady_clock::now() < startTime + std::chrono::milliseconds(500)
		? HttpResult::Code::Ok
		: HttpResult::Code::TooManyRequests;

	while (std::chrono::steady_clock::now() < startTime + std::chrono::seconds(1))
	{
		ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);
	}
	ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);

	std::this_thread::sleep_until(startTime + std::chrono::milliseconds(1500));
	ASSERT_EQUAL(limiter.ValidateRequest(), expectedEndResult);
}

namespace LimiterSpecs
{
	static constexpr int minRPS = 1;
	static constexpr int maxRPS = 100'000;
};

int main()
{
	try
	{
		TestWithPeakLoadInTheBeginning(LimiterSpecs::maxRPS);
		std::cout << "All Tests passed successfully\n";
	}
	catch (AssertionException& e)
	{
		std::cout << "One or more of tests failed: " << e.what() << '\n';
	}
	system("pause");
	return 0;
}
