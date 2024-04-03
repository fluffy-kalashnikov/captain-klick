#pragma once

class TimerScope
{
public:
    TimerScope() = delete;
    TimerScope(const char* aBenchmarkName);
    TimerScope(const TimerScope&) = delete;
    ~TimerScope();
    TimerScope& operator=(const TimerScope&) = delete;
private:
    const char* const myName;
    const long long myCount;
};