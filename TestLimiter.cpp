#include "Limiter.h"

auto TestLimiter(const Limiter& limiter)
{}

int main()
{
	const Limiter limiter(100'000, 1000);
	TestLimiter(limiter);
	return 0;
}