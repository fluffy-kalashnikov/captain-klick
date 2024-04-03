#include "pch.h"
#include "TimerScope.h"

static inline long long Count()
{
    LARGE_INTEGER count{};
    QueryPerformanceCounter(&count);
    return count.QuadPart;
}
static inline long long CountToMs(long long aPerformanceCount)
{
    static const long long COUNT_FREQUENCY = ([]()->long long {
        LARGE_INTEGER frequency{};
        QueryPerformanceFrequency(&frequency);
        return frequency.QuadPart;
        })();

    return aPerformanceCount * 1000LL / COUNT_FREQUENCY;
}

TimerScope::TimerScope(const char* aBenchmarkName)
    : myName(aBenchmarkName)
    , myCount(Count())
{
}

TimerScope::~TimerScope()
{
    const long long diff = Count() - myCount;
    OutputDebugStringA(myName);
    OutputDebugStringA(" took ");
    OutputDebugStringA(std::to_string(CountToMs(diff)).c_str());
    OutputDebugStringA("ms\n");
}