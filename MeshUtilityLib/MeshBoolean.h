#pragma once

#include "UtilityGlobal.h" 
#include "Plane.h"
#include "Box.h"

class MeshBoolean
{
public:
	enum OPERATOR{	UNION, DIFF, ISCT, XOR, RESOLVE	};

	static SurfaceMeshModel* cork(SurfaceMeshModel* m, Geom::Box box, OPERATOR op);

	static SurfaceMeshModel* cork(SurfaceMeshModel* m1, SurfaceMeshModel* m2, OPERATOR op);

private:
	static QString workPath;
	static QString boxName, appName;
};