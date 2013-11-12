#pragma once

#include "FoldabilizerLibGlobal.h"

#include <cstdlib>
#include <QTime>
#include <QDebug>

#define _USE_MATH_DEFINES
#include <math.h>

#include <random>

inline Vector3 minimize(const Vector3 a, const Vector3 b){
	Vector3 c = a;
	for (int i = 0; i < 3; i++)	
		if (b[i] < a[i]) c[i] = b[i];

	return c;
}

inline Vector3 maximize(const Vector3 a, const Vector3 b){
	Vector3 c = a;
	for (int i = 0; i < 3; i++)	
		if (b[i] > a[i]) c[i] = b[i];

	return c;
}

inline QVector<Vector3> XYZ()
{
	QVector<Vector3> a;
	a.push_back(Vector3(1, 0, 0));
	a.push_back(Vector3(0, 1, 0));
	a.push_back(Vector3(0, 0, 1));
	return a;
}


inline double uniformRealDistribution(double a = 0.0, double b = 1.0)
{
	double r = qrand() / (double) RAND_MAX;
	return (1 - r) * a + r * b;
}

inline int uniformDiscreteDistribution(int a, int b)
{
	double r = uniformRealDistribution();
	return a + int(r * (b - a));
}

inline int discreteDistribution(const QVector<double>& posibility)
{
	QVector<double> accPosibility;
	accPosibility.push_back(0);
	foreach (double p, posibility)	
		accPosibility.push_back(accPosibility.last() + p);

	double r = uniformRealDistribution();
	for (int i = 0; i < accPosibility.size()-1; i++)
	{
		if (r >= accPosibility[i] && r <= accPosibility[i+1])
			return i;
	}

	return 0;
}

inline double normalDistritution(double mean, double stddev)
{
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(mean, stddev);

	return distribution(generator);
}

inline double periodicalRanged(double a, double b, double v)
{
	double p = b -a;
	double n = (v - a) / p;
	if(n < 0)
		return v - p * int(n) + p;
	else
		return v - p * int(n);
}

inline double radians2degrees(double r)
{
	return 180 * r / M_PI;
}