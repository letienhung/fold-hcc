#include "Frame.h"
#include "Numeric.h"

Geom::Frame::Frame()
{ 
	c = Vec3d(0,0,0);
	r = Vec3d(1,0,0); 
	s = Vec3d(0,1,0); 
	t = Vec3d(0,0,1); 
}

Geom::Frame::Frame( const Vec3d& C,  const Vec3d& R, const Vec3d& S, const Vec3d& T )
{
	c = C;
	r = R.normalized(); 
	s = S.normalized(); 
	t = T.normalized(); 

	// make it right handed
	if (dot(cross(r, s), t) < 0) t *= -1;
}

Vector3 Geom::Frame::getCoordinates( Vector3 p )
{
	Vector3 v = p - c;
	return Vector3(dot(v, r), dot(v, s), dot(v, t));
}

Vector3 Geom::Frame::getPosition( Vector3 coord )
{
	Vector3 v = coord.x()*r + coord.y()*s + coord.z()*t;
	return c + v;
}

bool Geom::Frame::isAlignedWith( const Frame& other )
{
	return (areCollinear(r, other.r) || areCollinear(r, other.s) || areCollinear(r, other.t))
		&& (areCollinear(s, other.r) || areCollinear(s, other.s) || areCollinear(s, other.t));
}

Geom::Frame::RecordInFrame Geom::Frame::encodeFrame( Frame& other )
{
	RecordInFrame record;
	record.c = getCoordinates(other.c);
	record.cpr = getCoordinates(other.r + other.c);
	record.cps = getCoordinates(other.s + other.c);
	record.cpt = getCoordinates(other.t + other.c);

	return record;
}

Geom::Frame Geom::Frame::decodeFrame( RecordInFrame record )
{
	Geom::Frame frame;
	frame.c = getPosition(record.c);
	frame.r = getPosition(record.cpr) - frame.c;
	frame.s = getPosition(record.cps) - frame.c;
	frame.t = getPosition(record.cpt) - frame.c;

	return frame;
}
