#pragma once

#include "UtilityGlobal.h" 
#include "Plane.h"
#include "Box.h"

class MeshBoolean
{
public:
	enum OPERATOR{	UNION, DIFF, ISCT, XOR, RESOLVE	};

	static SurfaceMeshModel* getDifference(SurfaceMeshModel* m, Geom::Box cutBox);

	static SurfaceMeshModel* cork(SurfaceMeshModel* m1, SurfaceMeshModel* m2, OPERATOR op);

private:
	static QString workPath;
	static QString boxName, appName;
};