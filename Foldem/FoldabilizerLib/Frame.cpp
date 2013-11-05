#include "Frame.h"

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

Vec3d Frame::coordinates( const Vec3d& p )
{
	Vec3d v = p - c;
	return Vec3d(dot(v, r), dot(v, s), dot(v, t));
}

Vec3d Frame::position( const Vec3d& coord )
{
	Vec3d v = coord.x()*r + coord.y()*s + coord.z()*t;
	return c + v;
}
