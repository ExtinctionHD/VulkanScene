#pragma once

#include <chrono>

using namespace std::chrono;

// timer class for scene rendering
class Timer
{
public:
	Timer();
	~Timer();

	// returns difference between last time point and current time in seconds
	// and save current time as last time point
	float getDeltaSec();

private:
	time_point<steady_clock> lastTimePoint = time_point<steady_clock>::max();
};

