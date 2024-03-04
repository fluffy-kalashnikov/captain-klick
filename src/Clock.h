#pragma once

class Clock
{
public:
	Clock();
	Clock(const Clock&) = delete;
	~Clock() = default;
	Clock& operator=(const Clock&) = delete;
	void OnInitialize();
	void OnUpdate();
	float GetDeltaSeconds() const;
	float GetTimeSeconds() const;
private:
	long long myInitCount;
	long long myPrevCount;
	float myFrequencyCount;
	float myDeltaSeconds;
	float myTimeSeconds;
};




inline float Clock::GetDeltaSeconds() const
{
	return myDeltaSeconds;
}

inline float Clock::GetTimeSeconds() const
{
	return myTimeSeconds;
}