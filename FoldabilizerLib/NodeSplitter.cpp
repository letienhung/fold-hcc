#include "NodeSplitter.h"


QVector<Node*> NodeSplitter::uniformSplit( Node* node, int axisId, int N )
{
	double step = 2.0 / N;
	QVector<double> t;
	for (int i = 1; i < N; i++)
		t.push_back(-1 + i*step);

	return split(node, axisId, t);
}

QVector<Node*> NodeSplitter::split( Node* node, int axisId, QVector<double> t )
{
	const Vector3& old_center = node->mBox.Center;
	const QVector<Vector3> &old_axis = node->mBox.Axis;
	Vector3 old_extent = node->mBox.Extent;

	QVector<Node*> nodes;
	QVector<double> sp;	sp << -1 << t << 1;
	for (int i = 0; i < sp.size()-1; i++)
	{
		double c = (sp[i+1] + sp[i]) / 2;
		double e = (sp[i+1] - sp[i]) / 2;
		Vector3 center = old_center + c * old_axis[axisId];
		old_extent[axisId] = e;
		nodes.push_back(new Node(Geom::Box(center, old_axis, old_extent), 
									node->mID + QString::number(i)));
	}

	return nodes;
}