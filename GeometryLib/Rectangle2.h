#pragma once
#include "UtilityGlobal.h"
#include <QStringList>
#include "Segment2.h"

namespace Geom{

	class Rectangle2
	{
	public:
		Rectangle2();
		Rectangle2(Vector2 &center, QVector<Vector2> &axis, Vector2 &extent);
		Rectangle2(QVector<Vector2> &conners);
		
		// set
		void copyFrom();

		// geometry
		static int EDGE[4][2];
		double area();
		int getAxisId(Vector2& v);
		QVector<Vector2> getConners();
		QVector<Segment2> getEdgeSegments();

		// coordinates
		Vector2 getCoordinates(Vector2& p);
		Vector2 getPosition(Vector2& coord);
		bool contains(Vector2& p);

		// samples
		QVector<Vector2> getEdgeSamples(int N);

		// shrink
		void shrinkToAvoidPoints(QVector<Vector2>& pnts, Segment2& base);
		Rectangle2 shrinkFront(QVector<Vector2>& pnts, Segment2& base);
		Rectangle2 shrinkLeftRight(QVector<Vector2>& pnts, Segment2& base);
		Rectangle2 shrinkFrontLeftRight(QVector<Vector2>& pnts, Segment2& base);

		QStringList toStrList(); 

	public:
		Vector2 Center;
		QVector<Vector2> Axis;
		Vector2 Extent;

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	typedef Rectangle2 Box2;
}