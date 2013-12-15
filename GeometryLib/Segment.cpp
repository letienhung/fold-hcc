#include "Segment.h"
#include "Numeric.h"
#include "CustomDrawObjects.h"


Geom::Segment::Segment( Vector3 p0, Vector3 p1 )
{
	setFromEnds(p0, p1);
}


void Geom::Segment::computeCenterDirectionExtent()
{
	Vector3 d = P1 - P0;

	this->Center = (P0 + P1) / 2;
	this->Direction = d.normalized();
	this->Extent = d.norm()/2;
}



void Geom::Segment::setFromEnds( Vector3 p0, Vector3 p1 )
{
	P0 = p0;
	P1 = p1;

	computeCenterDirectionExtent();
}


bool Geom::Segment::isCollinearWith( const Segment& other )
{
	return this->isCollinearWith(other.P0)
		&& this->isCollinearWith(other.P1);
}

bool Geom::Segment::isCollinearWith( Vector3 p )
{
	Vector3 d = p - this->Center;

	if (d.norm() < ZERO_TOLERANCE_LOW) 
		return true;
	else
		return areCollinear(this->Direction, d);
}

bool Geom::Segment::overlaps( const Segment& other )
{

	if (!this->isCollinearWith(other))
		return false;

	//qDebug() << "Overlap test: ";
	//qDebug() << "\tThis seg: P0 = " << qstr(this->P0) << ", P1 = " << qstr(this->P1) << ", Direction = " << qstr(this->Direction);
	//qDebug() << "\tOther seg: P0 = " << qstr(other.P0) << ", P1 = " << qstr(other.P1) << ", Direction = " << qstr(other.Direction);
	//qDebug() << "\t***They are collinear.";

	int s0 = this->whichSide(other.P0);
	int s1 = this->whichSide(other.P1);
	Vector3 op0 = other.P0, op1 = other.P1;

	// flip other has opposite direction
	bool isFlip = false;
	if (dot(this->Direction, other.Direction) < 0)
	{
		isFlip = true;
		std::swap(s0, s1);
		std::swap(op0, op1);
	}

	// find
	if (  (s0 == SEG_NEGATIVE && s1 == SEG_NEGATIVE)
		|| s0 == SEG_POSITIVE && s1 == SEG_POSITIVE)
		return false;
	else
	{
		IT0 = (s0 == SEG_ON)? op0 : this->P0;
		IT1 = (s1 == SEG_ON)? op1 : this->P1;
		return true;
	}
}

double Geom::Segment::getProjectedCoordinate( Vector3 p )
{
	Vector3 pv = p - this->Center;
	return dot(pv, this->Direction) / Extent;
}

Vector3 Geom::Segment::getPosition( double coord )
{
	return Center +  coord * Extent * Direction;
}

int Geom::Segment::whichSide( Vector3 p )
{
	if (!this->isCollinearWith(p))
		return SEG_OFF;

	double c = this->getProjectedCoordinate(p);
	if (c <= -1) 
		return SEG_NEGATIVE;
	else if (c >= 1) 
		return SEG_POSITIVE;
	else 
		return SEG_ON;
}

bool Geom::Segment::contains( const Vector3 p )
{
	if (!this->isCollinearWith(p))
		return false;

	double c = this->getProjectedCoordinate(p);
	return c >= -1 && c <= 1;
}

bool Geom::Segment::contains( const Segment& other )
{
	return this->contains(other.P0) 
		&& this->contains(other.P1);
}

void Geom::Segment::computeEndPoints()
{
	P0 = Center - Extent * Direction;
	P1 = Center + Extent * Direction;
}

void Geom::Segment::draw(double width, QColor color)
{
	LineSegments ls(width);
	ls.addLine(P0, P1, color);
	ls.draw();
}
