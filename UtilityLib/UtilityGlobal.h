#pragma once

#include <QString>
#include <QtXml/QDomDocument>

#include <QVector>
#include <QQueue>
#include <QMap>
#include <QSet>

#include <QFile>

#include "SurfaceMeshModel.h"
using namespace SurfaceMesh;

namespace SurfaceMesh{
typedef Eigen::Vector2d Vector2;
typedef Eigen::Vector4d Vector4;
}

inline QVector<Vector3> getMeshVertices(SurfaceMeshModel *mesh)
{
	QVector<Vec3d> pnts;	

	Surface_mesh::Vertex_property<Point> points = mesh->vertex_property<Point>("v:point");
	Surface_mesh::Vertex_iterator vit, vend = mesh->vertices_end();

	for (vit = mesh->vertices_begin(); vit != vend; ++vit)
		pnts.push_back(points[vit]);

	return pnts;
}