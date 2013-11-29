#include "Frame.h"
#include "Numeric.h"

using namespace Geom;

Frame::Frame()
{ 
	c = Vec3d(0,0,0);
	r = Vec3d(1,0,0); 
	s = Vec3d(0,1,0); 
	t = Vec3d(0,0,1); 
}

Frame::Frame( const Vec3d& C,  const Vec3d& R, const Vec3d& S, const Vec3d& T )
{
	c = C;
	r = R; 
	s = S; 
	t = T; 
	normalize();
}

void Frame::normalize()
{
	r.normalize(); 
	s.normalize(); 
	t.normalize();
}

Vector3 Frame::getCoordinates( Vector3 p )
{
	Vector3 v = p - c;
	return Vector3(dot(v, r), dot(v, s), dot(v, t));
}

Vector3 Frame::getPosition( Vector3 coord )
{
	Vector3 v = coord.x()*r + coord.y()*s + coord.z()*t;
	return c + v;
}

bool Frame::isAlignedWith( const Frame& other )
{
	return (isCollinear(r, other.r) || isCollinear(r, other.s) || isCollinear(r, other.t))
		&& (isCollinear(s, other.r) || isCollinear(s, other.s) || isCollinear(s, other.t));
}
