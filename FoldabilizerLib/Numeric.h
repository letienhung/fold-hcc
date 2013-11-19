#pragma once

#include "FoldabilizerLibGlobal.h"

#include <cstdlib>
#include <QTime>
#include <QDebug>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

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

inline bool isCollinear(const Vector3& v0, const Vector3& v1)
{
	return cross(v0, v1).norm() < ZERO_TOLERANCE;
}