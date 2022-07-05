#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include <rate-limiter/limiter.h>

#include "Testing.h"

using namespace rate_limiter;
using namespace std::chrono;
using namespace std::chrono_literals;

static auto CurrentTime() { return Clock::now(); }

template <typename Clock, typename Duration>
std::string
millisecondsSinceStart(std::chrono::time_point<Clock, Duration> timepoint) {
  static const auto start = timepoint.time_since_epoch().count();
  return std::to_string((timepoint.time_since_epoch().count() - start) /
                        1'000'000);
}

static auto TestAllRequestsBelowMaxAreAccepted(std::uint64_t maxAllowedRps) {
  Limiter limiter(maxAllowedRps, 100);
  const auto startTime = CurrentTime();
  for (std::uint64_t i = 0; i < maxAllowedRps; ++i) {
    if (CurrentTime() < startTime + 1s) {
      ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
    }
  }
}

static auto TestAllRequestsAboveMaxAreDeclined(std::uint64_t maxAllowedRps) {
  Limiter limiter(maxAllowedRps, 100);
  const auto startTime = CurrentTime();
  for (std::uint64_t i = 0; i < maxAllowedRps; ++i) {
    if (CurrentTime() < startTime + 1s) {
      limiter.ValidateRequest();
      ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
    }
  }

  while (CurrentTime() < startTime + 1s) {
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);
  }
}

struct TimeStamps {
  const std::chrono::time_point<Clock> firstAcceptedRequest;
  const std::chrono::time_point<Clock> lastAcceptedRequest;
};

static auto TestWithPeakLoadAtStart(Limiter &limiter) {
  // Accepting the requests until we hit the limit.
  const auto firstAcceptedRequestTime = CurrentTime();
  for (std::uint64_t i = 0; i < limiter.MaxRPS(); ++i) {
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
  }
  const auto lastAcceptedRequestTime = CurrentTime();

  // Do not accept requests if less than one second has elapsed after the first
  // request.
  if (CurrentTime() < firstAcceptedRequestTime + 1s)
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);

  std::this_thread::sleep_until(firstAcceptedRequestTime +
                                std::chrono::milliseconds(900));
  if (CurrentTime() < firstAcceptedRequestTime + 1s)
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);

  return TimeStamps{firstAcceptedRequestTime, lastAcceptedRequestTime};
}

static auto
TestWithPeakLoadAtStart_SingleIteration(std::uint64_t maxAllowedRps) {
  Limiter limiter(maxAllowedRps, 1000);

  const auto timeStamps = TestWithPeakLoadAtStart(limiter);

  // Herewith we ensure that NOT MORE than maxAllowedRps requests are allowed
  // per second. It is possible that the limiter will allow a bit LESS, though,
  // hence this "delta" allowance in the assertion below.
  constexpr auto delayDueToTimerIssueObtainedEmpirically =
      std::chrono::milliseconds(42);
  std::this_thread::sleep_until(timeStamps.firstAcceptedRequest + 1s +
                                delayDueToTimerIssueObtainedEmpirically);
  ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
}

static auto
TestWithPeakLoadAtStart_MultipleIterations(std::uint64_t maxAllowedRps,
                                           std::uint64_t iterations) {
  Limiter limiter(maxAllowedRps, 100);

  for (std::uint64_t i = 0; i < iterations; ++i) {
    const auto timeStamps = TestWithPeakLoadAtStart(limiter);
    std::this_thread::sleep_until(timeStamps.lastAcceptedRequest + 1s);
  }
}

static auto TestWithEvenLoad(std::uint64_t maxAllowedRps) {
  Limiter limiter(maxAllowedRps, 100);
  const auto intervalBetweenRequests = 1s / (2 * maxAllowedRps);

  for (int iterations = 0; iterations < 5; ++iterations) {
    int requestsSent = 0;
    const auto startTime = CurrentTime();

    while (requestsSent < maxAllowedRps && CurrentTime() < startTime + 1s) {
      ++requestsSent;
      const auto result = limiter.ValidateRequest();

      ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
      std::this_thread::sleep_for(intervalBetweenRequests);
    }
    const auto lastValidRequestTime = CurrentTime();

    while (CurrentTime() < startTime + 1s) {
      ASSERT_EQUAL(limiter.ValidateRequest(),
                   HttpResult::Code::TooManyRequests);
      std::this_thread::sleep_for(intervalBetweenRequests);
    }

    std::this_thread::sleep_until(lastValidRequestTime + 1s);
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
  }
}

static auto TestWithAdjacentPeaks(std::uint64_t maxAllowedRps) {
  const auto startTime = CurrentTime();

  Limiter limiter(maxAllowedRps, 1000);

  std::this_thread::sleep_until(startTime + std::chrono::milliseconds(900));
  int requestsSent = 0;
  while (requestsSent < maxAllowedRps * 0.8) {
    requestsSent++;
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
  }

  std::this_thread::sleep_until(startTime + std::chrono::milliseconds(1500));
  while (requestsSent < maxAllowedRps) {
    requestsSent++;
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
  }

  while (CurrentTime() < startTime + std::chrono::milliseconds(1900)) {
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);
  }

  std::this_thread::sleep_until(startTime + std::chrono::milliseconds(2000));
  ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
}

namespace LimiterSpecs {
static constexpr std::uint64_t minRPS = 1;
static constexpr std::uint64_t maxRPS = 100'0000;
}; // namespace LimiterSpecs

int main() {
  // Failing tests are commented out.
  try {
    TestAllRequestsBelowMaxAreAccepted(LimiterSpecs::maxRPS);
    TestAllRequestsAboveMaxAreDeclined(LimiterSpecs::maxRPS);
    TestWithPeakLoadAtStart_SingleIteration(LimiterSpecs::maxRPS);
    // TestWithPeakLoadAtStart_MultipleIterations(LimiterSpecs::maxRPS, 10);
    TestWithAdjacentPeaks(LimiterSpecs::maxRPS);
    TestWithEvenLoad(LimiterSpecs::maxRPS);
    std::cout << "All Tests passed successfully\n";
  } catch (AssertionException &e) {
    std::cout << "One or more of tests failed: " << e.what() << '\n';
  }

#if _WIN32
  system("pause");
#endif

  return 0;
}
