#pragma once

class TimeInterval
{
public:
    TimeInterval(double t0 = 0.0, double t1 = 1.0);

	bool overlaps(TimeInterval other);
	bool contains(double t);
	bool contains(TimeInterval other);
	bool hasPassed(double t);
	bool hasReached(double t);

	double getLocalTime(double t);
	double getDuration();

	void set(double t0, double t1);

public:
	double start;
	double end;
};

