#pragma once

#include "FoldabilizerLibGlobal.h"

#include <cstdlib>
#include <QTime>
#include <QDebug>

#define _USE_MATH_DEFINES
#include <math.h>

#include <random>

// utility macros
#define Max(a,b) (((a) > (b)) ? (a) : (b))
#define Min(a,b) (((a) < (b)) ? (a) : (b))
#define RANGED(min, v, max) ( Max(min, Min(v, max)) ) 

// Tolerance
#define ZERO_TOLERANCE 1e-06

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
	r =  (1 - r) * a + r * b;
	return RANGED(a, r, b);
}

inline int uniformDiscreteDistribution(int a, int b)
{
	double r = uniformRealDistribution();
	int n = a + int(r * (b - a));
	return RANGED(a, n, b-1);
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


// Marsaglia polar method
// Wikipedia: http://en.wikipedia.org/wiki/Marsaglia_polar_method
class NormalDistribution
{
public:
	NormalDistribution(){isSpareReady = false;}
	~NormalDistribution(){}
	double generate(double mean, double stdDev ){
		if (isSpareReady) {
			isSpareReady = false;
			return spare * stdDev + mean;
		} else {
			double u, v, s;
			do {
				u = uniformRealDistribution() * 2 - 1;
				v = uniformRealDistribution() * 2 - 1;
				s = u * u + v * v;
			} while (s >= 1 || s == 0);
			double mul = sqrt(-2.0 * log(s) / s);
			spare = v * mul;
			isSpareReady = true;
			return mean + stdDev * u * mul;
		}
	}
private:
	double	spare;
	bool	isSpareReady;
};