#include "PcaOBB.h"
#include "PCA.h"
#include "Frame.h"
#include "AABB.h"
#include "MinOBB2.h"
#include "Rectangle.h"
#include "Rectangle2.h"
#include "Numeric.h"

#include <fstream>

Geom::PcaOBB::PcaOBB( QVector<Vector3>& pnts)
{
	// fit pca box
	PCA pca(pnts);
	Geom::Frame frame(pca.mu, pca.eigenVectors[0], pca.eigenVectors[1], pca.eigenVectors[2]);
	QVector<Vector3> coords;
	foreach(Vector3 p, pnts) coords << frame.getCoordinates(p);
	Geom::AABB aabb(coords);
	frame.c = frame.getPosition(aabb.center());
	Vector3 extent = (aabb.bbmax - aabb.bbmin) * 0.5;
	Geom::Box pcaBox = Geom::Box(frame, extent);
	
	// project points on to base plane
	// and find minOBB2
	Geom::Rectangle baseRect = pcaBox.getPatch(0, -1);
	Geom::Plane basePlane = baseRect.getPlane();
	QVector<Vector2> pnts2; 
	double maxHeight = minDouble();
	foreach(Vector3 p, pnts) 
	{
		double h = basePlane.distanceTo(p);
		if (h > maxHeight) maxHeight = h;

		pnts2 << baseRect.getOpenProjCoord(p);
	}

	MinOBB2 obb2(pnts2);
	Geom::Rectangle2 rect2 = obb2.getBox2();

	// recover minOBB
	Vector3 c = baseRect.getOpenPos(rect2.Center) + 0.5 * maxHeight * baseRect.Normal;
	QVector<Vector3> axis;
	axis << baseRect.getOpenVector(rect2.Axis[0]) << baseRect.getOpenVector(rect2.Axis[1])
		 << baseRect.Normal;
	Vector3 ext(rect2.Extent[0], rect2.Extent[1], 0.5 * maxHeight);

	minBox = Geom::Box(c, axis, ext);
}
