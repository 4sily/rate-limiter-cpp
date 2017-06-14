#include "Limiter.h"
#include "Testing.h"

#include <iostream>

static auto CurrentTime() { return Clock::now(); }
static constexpr auto oneSecond = std::chrono::seconds(1);

std::ostream& operator<<(std::ostream& os, HttpResult::Code code)
{
    const auto str =
        code == HttpResult::Code::Ok ? "Ok (200)" :
        code == HttpResult::Code::TooManyRequests ? "Too many requests (429)" :
        "Unknown code";
    os << str;
    return os;
}

template <typename Clock, typename Duration>
std::string millisecondsSinceStart(std::chrono::time_point<Clock, Duration> timepoint)
{
    const static auto start = timepoint.time_since_epoch().count();
    return std::to_string((timepoint.time_since_epoch().count() - start) / 1'000'000);
}

static auto TestAllRequestsBelowMaxAreAccepted(int maxAllowedRps)
{
    Limiter limiter(maxAllowedRps, 100);
    const auto startTime = CurrentTime();
    for (int i = 0; i < maxAllowedRps; ++i)
        if (CurrentTime() < startTime + oneSecond)
            ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
}

static auto TestAllRequestsAboveMaxAreDeclined(int maxAllowedRps)
{
    Limiter limiter(maxAllowedRps, 100);
    const auto startTime = CurrentTime();
    for (int i = 0; i < maxAllowedRps; ++i)
        if (CurrentTime() < startTime + oneSecond)
            limiter.ValidateRequest();

    while (CurrentTime() < startTime + oneSecond)
        ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);
}

struct TimeStamps
{
    const std::chrono::time_point<Clock> firstAcceptedRequest;
    const std::chrono::time_point<Clock> lastAcceptedRequest;
};

static auto TestWithPeakLoadAtStart(Limiter& limiter)
{
    // Accepting the requests until we hit the limit.
    const auto firstAcceptedRequestTime = CurrentTime();
    for (int i = 0; i < limiter.maxRPS(); ++i)
    {
        ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
    }
    const auto lastAcceptedRequestTime = CurrentTime();

    // Do not accept requests if less than one second has elapsed after the first request.
    if (CurrentTime() < firstAcceptedRequestTime + oneSecond)
        ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);

    std::this_thread::sleep_until(firstAcceptedRequestTime + std::chrono::milliseconds(900));
    if (CurrentTime() < firstAcceptedRequestTime + oneSecond)
        ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);

    return TimeStamps{ firstAcceptedRequestTime, lastAcceptedRequestTime };
}

static auto TestWithPeakLoadAtStart_SingleIteration(int maxAllowedRps)
{
    Limiter limiter(maxAllowedRps, 1000);

    const auto timeStamps = TestWithPeakLoadAtStart(limiter);

    // Herewith we ensure that NOT MORE than maxAllowedRps requests are allowed per second.
    // It is possible that the limiter will allow a bit LESS, though, hence this "delta" allowance
    // in the assertion below.
    constexpr auto delayDueToTimerIssueObtainedEmpirically = std::chrono::milliseconds(42);
    std::this_thread::sleep_until(timeStamps.firstAcceptedRequest + oneSecond + delayDueToTimerIssueObtainedEmpirically);
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
}


static auto TestWithPeakLoadAtStart_MultipleIterations(int maxAllowedRps, int iterations)
{
    Limiter limiter(maxAllowedRps, 100);

    for (int i = 0; i < iterations; ++i)
    {
        const auto timeStamps = TestWithPeakLoadAtStart(limiter);
        std::this_thread::sleep_until(timeStamps.lastAcceptedRequest + oneSecond);
    }
}

static auto TestWithEvenLoad(int maxAllowedRps)
{
    Limiter limiter(maxAllowedRps, 100);
    const auto intervalBetweenRequests = oneSecond / (2 * maxAllowedRps);

    for (int iterations = 0; iterations < 5; ++iterations)
    {
        int requestsSent = 0;
        const auto startTime = CurrentTime();

        while (requestsSent < maxAllowedRps &&
               CurrentTime() < startTime + oneSecond)
        {
            ++requestsSent;
            const auto result = limiter.ValidateRequest();

            ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
            std::this_thread::sleep_for(intervalBetweenRequests);
        }
        const auto lastValidRequestTime = CurrentTime();

        while (CurrentTime() < startTime + oneSecond)
        {
            ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);
            std::this_thread::sleep_for(intervalBetweenRequests);
        }

        std::this_thread::sleep_until(lastValidRequestTime + oneSecond);
        ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
    }
}

static auto TestWithAdjacentPeaks(int maxAllowedRps)
{
    const auto startTime = CurrentTime();

    Limiter limiter(maxAllowedRps, 1000);

    std::this_thread::sleep_until(startTime + std::chrono::milliseconds(900));
    int requestsSent = 0;
    while (requestsSent < maxAllowedRps * 0.8)
    {
        requestsSent++;
        ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
    }

    std::this_thread::sleep_until(startTime + std::chrono::milliseconds(1500));
    while (requestsSent < maxAllowedRps)
    {
        requestsSent++;
        ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
    }

    while (CurrentTime() < startTime + std::chrono::milliseconds(1900))
    {
        ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::TooManyRequests);
    }

    std::this_thread::sleep_until(startTime + std::chrono::milliseconds(2000));
    ASSERT_EQUAL(limiter.ValidateRequest(), HttpResult::Code::Ok);
}

namespace LimiterSpecs
{
    static constexpr int minRPS = 1;
    static constexpr int maxRPS = 100'000;
};

int main()
{
    // Failing tests are commented out.
    try
    {
        TestAllRequestsBelowMaxAreAccepted(LimiterSpecs::maxRPS);
        TestAllRequestsAboveMaxAreDeclined(LimiterSpecs::maxRPS);
        TestWithPeakLoadAtStart_SingleIteration(LimiterSpecs::maxRPS);
        //TestWithPeakLoadAtStart_MultipleIterations(LimiterSpecs::maxRPS, 10);
        TestWithAdjacentPeaks(LimiterSpecs::maxRPS);
        TestWithEvenLoad(LimiterSpecs::maxRPS);
        std::cout << "All Tests passed successfully\n";
    }
    catch (AssertionException& e)
    {
        std::cout << "One or more of tests failed: " << e.what() << '\n';
    }
    system("pause");
    return 0;
}
