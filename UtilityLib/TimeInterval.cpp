#include "TimeInterval.h"
#include "Numeric.h"

TimeInterval::TimeInterval(double t0, double t1)
{
	set(t0, t1);
}

bool TimeInterval::overlaps( TimeInterval other)
{
	return this->contains(other.start) || this->contains(other.end) ||
		other.contains(this->start) || other.contains(this->end);
}

bool TimeInterval::contains(double t)
{
	return inRange(t, start, end);
}

bool TimeInterval::contains(TimeInterval other)
{
	return contains(other.start) && contains(other.end);
}

bool TimeInterval::hasPassed(double t)
{
	return t >= this->end;
}

double TimeInterval::getLocalTime(double t)
{
	double locT = -1;
	if (contains(t)) locT = (t - start) / getDuration();
	else if (hasPassed(t)) locT = 1;
	else locT = 0;

	return locT;
}

double TimeInterval::getDuration()
{
	return end - start;
}

void TimeInterval::set(double t0, double t1)
{
	if (t0 <= t1)
	{
		start = t0; 
		end = t1;	
	}
	else 
	{ 
		start = t1; 
		end = t0; 
	}
}
