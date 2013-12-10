#include "UtilityGlobal.h"

QVector<Vector3> getMeshVertices( SurfaceMeshModel *mesh )
{
	QVector<Vec3d> pnts;	

	Surface_mesh::Vertex_property<Point> points = mesh->vertex_property<Point>("v:point");
	Surface_mesh::Vertex_iterator vit, vend = mesh->vertices_end();

	for (vit = mesh->vertices_begin(); vit != vend; ++vit)
		pnts.push_back(points[vit]);

	return pnts;
}

QString qStr( Vector3 v, char sep)
{
	return QString("%1%1%1%1%1").arg(v.x()).arg(sep).arg(v.y()).arg(sep).arg(v.z());
}

QString qStr( const Vector4 &v, char sep )
{
	return QString("%1 %2 %3 %4").arg(v[0]).arg(v[1]).arg(v[2]).arg(v[3]);
}
