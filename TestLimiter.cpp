#include "Limiter.h"
#include "Testing.h"

#include <iostream>

auto TestWithPeakLoadInTheBeginning(int maxAllowedRps)
{
	const auto startTime = std::chrono::steady_clock::now();
	int sentRequestsCount = 0;

	Limiter limiter(maxAllowedRps, 100);
	for (; sentRequestsCount < maxAllowedRps; ++sentRequestsCount)
	{
		ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
	}
	if (std::chrono::steady_clock::now() < startTime + std::chrono::seconds())
	{
		ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);
	}
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
