#include "GeomUtility.h"
#include "Frame2.h"

Geom::Rectangle2 Geom::computeAABB(QVector<Vector2> &pnts)
{
	// compute extent along x and y
	double minX = maxDouble();
	double maxX = -maxDouble();
	double minY = maxDouble();
	double maxY = -maxDouble();
	for (auto p : pnts)
	{
		if (p.x() < minX) minX = p.x();
		if (p.x() > maxX) maxX = p.x();

		if (p.y() < minY) minY = p.y();
		if (p.y() > maxY) maxY = p.y();
	}

	// create rect
	QVector<Vector2> conners;
	conners << Vector2(minX, minY) << Vector2(maxX, minY) << Vector2(maxX, maxY) << Vector2(minX, maxY);
	return Rectangle2(conners);
}

Geom::Rectangle2 Geom::computeBoundingBox(QVector<Vector2>& pnts, Vector2& X)
{
	// assert
	if (pnts.isEmpty()) return Rectangle2();

	// encode points in frame B :centered at the 1st point with axis X
	Frame2 frameB(pnts.front(), X);
	QVector<Vector2> pntsB;
	for (Vector2 p : pnts) pntsB << frameB.getCoords(p);
	Rectangle2 aabbB = computeAABB(pntsB);

	// get back to frame A : where pnts lay 
	QVector<Vector2> connersA;
	for (Vector2 pB : aabbB.getConners()) 
		connersA << frameB.getPosition(pB);

	// result
	return Rectangle2(connersA);
}
