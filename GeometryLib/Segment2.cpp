#include "Segment2.h"

using namespace Geom;

Segment2::Segment2()
{
}

Segment2::Segment2(const Vector2& p0, const Vector2& p1)
{
	set(p0, p1);
}

QVector<Vector2> Geom::Segment2::getUniformSamples( int N )
{
	QVector<Vector2> samples;
	double step = 2 * Extent / (N - 1);
	for (int i = 0; i < N; i++)
		samples << P0 + i * step * Direction;

	return samples;
}

double Geom::Segment2::getProjCoordinates( Vector2& p )
{
	Vector2 c2p = p - Center;
	return dot(c2p, Direction) / Extent;
}

SurfaceMesh::Vector2 Geom::Segment2::getPosition( double t )
{
	return Center +  t * Extent * Direction;
}

void Geom::Segment2::set(const Vector2& p0, const Vector2& p1 )
{
	P0 = p0;
	P1 = p1;
	Center = (P0 + P1) / 2;
	Direction = P1 - P0;
	Extent = Direction.norm() / 2;
	Direction.normalize();
}

void Geom::Segment2::flip()
{
	Vector2 p0 = P1;
	Vector2 p1 = P0;
	set(p0, p1);
}
