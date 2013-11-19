#include "Line.h"
#include "Numeric.h"

Line::Line()
{
	Origin = Vector3(0, 0, 0);
	Direction = Vector3(1, 0, 0);
}

Line::Line( Vector3 o, Vector3 d )
	:Origin(o), Direction(d)
{
	Direction.normalize();
}

SurfaceMesh::Vector3 Line::getPoint( double t )
{
	return Origin + t * Direction;
}

bool Line::contains( Vector3 p )
{
	return isCollinear(this->Direction, p - Origin);
}

