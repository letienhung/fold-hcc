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
typedef Eigen::Vector2i Vector2i;
typedef Eigen::Vector2d Vector2;
typedef Eigen::Vector4d Vector4;
}

typedef QSharedPointer<SurfaceMeshModel> MeshPtr;

QString qStr(const Vector2 &v, char sep = ' ');
QString qStr(const Vector3 &v, char sep = ' ');
QString qStr(const Vector4 &v, char sep = ' ');

Vector3 toVector3(QString string);

typedef QVector< QVector<QString> > StrArray2D;

typedef QMap< QString, QVariant > PropertyMap;

#ifdef Q_OS_WIN
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
QString getcwd();