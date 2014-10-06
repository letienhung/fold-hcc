#include "Frame2.h"

Geom::Frame2::Frame2()
: Frame2(Vector2(0, 0))
{
}

Geom::Frame2::Frame2(Vector2& C)
: c(C)
{
	r = Vector2(1, 0);
	s = Vector2(0, 1);
}

Geom::Frame2::Frame2(Vector2& C, Vector2& X)
: Frame2(C)
{
	r = X.normalized();
	s = Vector2(r.y(), r.x());
}

Geom::Frame2::Frame2(Vector2& C, Vector2& X, Vector2& Y)
: Frame2(C, X)
{
	if (dot(s, Y) < 0)
		s *= -1;
}

SurfaceMesh::Vector2 Geom::Frame2::getCoords(Vector2& p)
{
	Vector2 v = p - c;
	return Vector2(dot(v, r), dot(v, s));
}

SurfaceMesh::Vector2 Geom::Frame2::getPosition(Vector2& coord)
{
	Vector2 v = coord.x()*r + coord.y()*s;
	return c + v;
}
