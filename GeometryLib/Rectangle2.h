#pragma once
#include "UtilityGlobal.h"
#include <QStringList>

namespace Geom{

	class Rectangle2
	{
	public:
		Rectangle2();
		Rectangle2(Vector2 &center, QVector<Vector2> &axis, Vector2 &extent);
		Rectangle2(QVector<Vector2> &conners);
		
		QVector<Vector2> getConners();
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