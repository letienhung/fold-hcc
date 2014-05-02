#pragma once

#include "UtilityGlobal.h"
#include "Segment.h"
#include "Box.h"
#include "SectorCylinder.h"

class FdNode;
class PatchNode;

typedef QVector< QVector<FdNode*> > FdNodeArray2D;

// Qt meta type
Q_DECLARE_METATYPE(Geom::SectorCylinder)
Q_DECLARE_METATYPE(Geom::Rectangle2)
Q_DECLARE_METATYPE(QVector<Geom::Segment>)

// distance between fd nodes
Geom::Segment getDistSegment( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n, QVector<FdNode*> nset);

// relation among fd nodes
FdNodeArray2D clusterNodes( QVector<FdNode*> nodes, double disThr );
QVector<Geom::Segment> detectJointSegments(FdNode* part, PatchNode* panel);
bool hasIntersection(FdNode* slave, PatchNode* master, double thr);

// helpers
StrArray2D getIds(FdNodeArray2D nodeArray);

// temporal relations
double getLocalTime(double globalT, double localStart, double localEnd);
QVector<double> getEvenDivision(int n, double start = 0, double end = 1);

// relation with plan
enum PLANE_RELATION{ON_PLANE, POS_PLANE, NEG_PLANE, ISCT_PLANE};
PLANE_RELATION relationWithPlane(FdNode* n, Geom::Plane plane, double thr);
bool onPlane( FdNode* n, Geom::Plane& plane );

// box fitting
enum BOX_FIT_METHOD{FIT_AABB, FIT_MIN, FIT_PCA};
Geom::Box fitBox(QVector<Vector3>& pnts, BOX_FIT_METHOD method = FIT_PCA);

// bundle nodes
QString getBundleName(const QVector<FdNode*>& nodes);
Geom::Box getBundleBox(const QVector<FdNode*>& nodes);

// tags
#define	IS_MASTER "isMasterPatch"
#define IS_EDGE_ROD "isEdgeRodNode"
#define IS_CONNECTIVITY "isConnectivity"