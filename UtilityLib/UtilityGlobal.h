#pragma once

#include "XmlWriter.h"
#include <QtXml/QDomDocument>

#include <QSharedPointer>
#include <QString>
#include <QVector>
#include <QQueue>
#include <QMap>
#include <QSet>

#include "SurfaceMeshModel.h"
using namespace SurfaceMesh;

namespace SurfaceMesh{
typedef Eigen::Vector2d Vector2;
typedef Eigen::Vector4d Vector4;
}

typedef QSharedPointer<SurfaceMeshModel> MeshPtr;

QVector<Vector3> getMeshVertices(SurfaceMeshModel* mesh);

QString qStr(Vector3 v, char sep = ' ');
QString qStr(const Vector4 &v, char sep = ' ');

Vector3 toVector3(QString string);