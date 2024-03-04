#include "pch.h"
#include "Clock.h"


Clock::Clock()
	: myInitCount(0)
	, myPrevCount(0)
	, myFrequencyCount(0)
	, myDeltaSeconds(0)
	, myTimeSeconds(0)
{
}

void Clock::OnInitialize()
{
	LARGE_INTEGER count{};
	QueryPerformanceFrequency(&count);
	myFrequencyCount = static_cast<float>(count.QuadPart);
	
	QueryPerformanceCounter(&count);
	myInitCount = count.QuadPart;
	myPrevCount = count.QuadPart;
}

void Clock::OnUpdate()
{
	LARGE_INTEGER count{};
	QueryPerformanceCounter(&count);

	myDeltaSeconds = (count.QuadPart - myPrevCount) / myFrequencyCount;
	myTimeSeconds = (count.QuadPart - myInitCount) / myFrequencyCount;
	myPrevCount = count.QuadPart;
}