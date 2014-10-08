#include "AABB.h"
#include "Numeric.h"
#include "UtilityGlobal.h"

Geom::AABB::AABB()
{
	bbmin = Point( FLT_MAX,  FLT_MAX,  FLT_MAX);
	bbmax = Point(-FLT_MAX, -FLT_MAX, -FLT_MAX);    
}

Geom::AABB::AABB( QVector<Vector3>& pnts )
{
	bbmin = Point( FLT_MAX,  FLT_MAX,  FLT_MAX);
	bbmax = Point(-FLT_MAX, -FLT_MAX, -FLT_MAX);    
	add(pnts);       
}

SurfaceMesh::Vector3 Geom::AABB::center()
{
	return (bbmin + bbmax) * 0.5f;
}

double Geom::AABB::radius()
{
	return (bbmax - bbmin).norm() * 0.5f;
}

Geom::Box Geom::AABB::box()
{
	Box b;
	b.Center = center();
	b.Axis = XYZ();
	b.Extent = (bbmax - bbmin) * 0.5f;

	return b;
}

void Geom::AABB::add( QVector<Vector3>& pnts )
{
	for (Point p : pnts) {
		bbmin = minimize(bbmin, p);
		bbmax = maximize(bbmax, p);
	} 
}

void Geom::AABB::add( AABB& other )
{
	bbmin = minimize(bbmin, other.bbmin);
	bbmax = maximize(bbmax, other.bbmax);
}

bool Geom::AABB::isValid()
{
	for (int i = 0; i < 3; i++)
	{
		if (bbmax[i] <= bbmin[i])
			return false;
	}

	return true;
}

void Geom::AABB::validate()
{
	if (!isValid())
	{
		bbmin = Vector3(-1,-1,-1);
		bbmax = Vector3( 1, 1, 1);
	}
}
