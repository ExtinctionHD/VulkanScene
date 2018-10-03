#include "Timer.h"

Timer::Timer()
{
}

Timer::~Timer()
{
}

double Timer::getDeltaSec()
{
	double deltaSec = 0;

	time_point<steady_clock> now = high_resolution_clock::now();

	if (lastTimePoint != time_point<steady_clock>::max())
	{
		deltaSec = duration_cast<milliseconds>(now - lastTimePoint).count() / 1000.0;
	}

	lastTimePoint = now;

	return deltaSec;
}
