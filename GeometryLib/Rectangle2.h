#pragma once
#include "UtilityGlobal.h"
#include <QStringList>
#include "Segment2.h"
#include "Numeric.h"

namespace Geom{

	class Rectangle2
	{
	public:
		Rectangle2();
		Rectangle2(Vector2 &center, QVector<Vector2> &axis, Vector2 &extent);
		Rectangle2(QVector<Vector2> &conners);

		// geometry
		static int EDGE[4][2];
		double	area();
		int		getAxisId(Vector2& v);
		int		getPerpAxisId(Vector2& v);
		double	getExtent(Vector2& v);
		double	getPerpExtent(Vector2& v);
		Vector2 getEdgeCenter(int aid, bool positive);
		Segment2 getSkeleton(int aid);
		QVector<Vector2>	getConners();
		QVector<Segment2>	getEdgeSegments();

		// coordinates
		Vector2 getCoordinates(Vector2& p);
		Vector2 getPosition(Vector2& coord);

		// relation with others
		bool contains(Vector2& p, double thr = ZERO_TOLERANCE_LOW);
		bool containsAll(QVector<Vector2>& pnts, double thr = ZERO_TOLERANCE_LOW);
		bool containsAny(QVector<Vector2>& pnts, double thr = ZERO_TOLERANCE_LOW);

		// samples
		QVector<Vector2> getEdgeSamples(int N);
		QVector<Vector2> getGridSamples(double w);

		// shrink
		void shrinkToExclude(QVector<Vector2>& pnts, Segment2& base);
		void shrinkFront(QVector<Vector2>& pnts, Segment2& base);
		void shrinkLeftRight(QVector<Vector2>& pnts, Segment2& base);
		void shrinkFrontLeftRight(QVector<Vector2>& pnts, Segment2& base);

		// expand
		void expandToTouch(QVector<Vector2>& pnts, double thr = ZERO_TOLERANCE_LOW);
		void expandAlongSingleAxisToTouch(QVector<Vector2>& pnts, int aid, double thr = ZERO_TOLERANCE_LOW);

		// crop
		void cropByAxisAlignedRectangle(Rectangle2& cropper);

		// 2D bounding box
		static Rectangle2 computeBoundingBox(QVector<Vector2>& pnts, Vector2 X);

		// to string
		QStringList toStrList(); 

	public:
		Vector2 Center;
		QVector<Vector2> Axis;
		Vector2 Extent;

	public:
		static Rectangle2 computeAABB(QVector<Vector2> &pnts);

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	typedef Rectangle2 Box2;
}