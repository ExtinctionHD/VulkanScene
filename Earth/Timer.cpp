#include "Timer.h"

Timer::Timer()
{
}

Timer::~Timer()
{
}

float Timer::getDeltaSec()
{
	float deltaSec = 0;

	time_point<steady_clock> now = high_resolution_clock::now();

	if (lastTimePoint != time_point<steady_clock>::max())
	{
		deltaSec = duration_cast<milliseconds>(now - lastTimePoint).count() / 1000.0f;
	}

	lastTimePoint = now;

	return deltaSec;
}
