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
#define ZERO_TOLERANCE_LOW 1e-06
#define ZERO_TOLERANCE_HIGH 1e-01

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

inline QString qstr(Vector3 v)
{
	return QString("(%1, %2, %3)").arg(v.x()).arg(v.y()).arg(v.z());
}

inline bool isCollinear(const Vector3& v0, const Vector3& v1)
{
	double cp = cross(v0, v1).norm();
	bool isCol = cp < ZERO_TOLERANCE_LOW;
	
	//qDebug() << "Collinearity checking:" << qstr(v0) << "and" << qstr(v1) << ": cross_normal = " << cp << ", isCol = "<< isCol;

	return isCol;
}

inline bool isPerp(const Vector3& v0, const Vector3& v1)
{
	return fabs(dot(v0, v1)) < ZERO_TOLERANCE_LOW;
}

