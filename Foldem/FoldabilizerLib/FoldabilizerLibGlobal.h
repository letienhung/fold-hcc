#pragma once

#include <QVector>
#include <QString>
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