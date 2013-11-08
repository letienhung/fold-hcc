#pragma once

#include <QVector>
#include <QString>
#include <QQueue>
#include <QtXml/QDomDocument>
#include <QFile>

#include "SurfaceMeshModel.h"
using namespace SurfaceMesh;

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