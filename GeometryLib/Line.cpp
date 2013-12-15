#include "Line.h"
#include "Numeric.h"

Geom::Line::Line()
{
	Origin = Vector3(0, 0, 0);
	Direction = Vector3(1, 0, 0);
}

Geom::Line::Line( Vector3 o, Vector3 d )
	:Origin(o), Direction(d)
{
	Direction.normalize();
}

SurfaceMesh::Vector3 Geom::Line::getPoint( double t )
{
	return Origin + t * Direction;
}

bool Geom::Line::contains( Vector3 p )
{
	return areCollinear(this->Direction, p - Origin);
}

