#pragma once

#include "UtilityGlobal.h"
#include "Segment.h"
#include "Box.h"
#include "SectorCylinder.h"

class FdNode;
class PatchNode;
class FdGraph;

// typedef
typedef QVector< QVector<FdNode*> > FdNodeArray2D;
typedef QPair<double, double> TimeInterval;
#define	TIME_INTERVAL(a, b) qMakePair<double, double>(a, b)

// tags
#define	MASTER_TAG "isMasterPatch"
#define EDGE_ROD_TAG "isEdgeRodNode"
#define DELETED_TAG "hasDeleted"
#define FOLDED_TAG "hasFolded"
#define READY_TAG "isReady"
#define SELECTED_TAG "isSelected"
#define NEW_TAG "isNew"

// Qt meta type
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(QVector<Vector3>)
Q_DECLARE_METATYPE(Geom::SectorCylinder)
Q_DECLARE_METATYPE(QVector<Geom::SectorCylinder>)
Q_DECLARE_METATYPE(Geom::Rectangle)
Q_DECLARE_METATYPE(QVector<Geom::Segment>)
Q_DECLARE_METATYPE(QVector<Geom::Box>)
Q_DECLARE_METATYPE(QVector<FdGraph*>)

// distance between fd nodes
Geom::Segment getDistSegment( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n, QVector<FdNode*> nset);

// relation among fd nodes
FdNodeArray2D getConnectedGroups( QVector<FdNode*> nodes, double disThr );
QVector<Geom::Segment> detectJointSegments(FdNode* part, PatchNode* panel);
bool hasIntersection(FdNode* slave, PatchNode* master, double thr);

// helpers
StrArray2D getIds(FdNodeArray2D nodeArray);

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

// time intervals
bool overlap(TimeInterval itv1, TimeInterval itv2);
bool within(double t, TimeInterval itv);
bool passed(double t, TimeInterval itv);
double getLocalTime(double globalT, TimeInterval itv);

// masters
QVector<PatchNode*> getAllMasters(FdGraph* scaffold);
QVector<QString> getAllMasterIds(FdGraph* scaffold);
int nbMasters(FdGraph* scaffold);
QMap<QString, double> getTimeStampsNormalized(QVector<FdNode*> nodes, Vector3 v, double &tScale);
QMap<QString, double> getTimeStampsNormalized(QVector<PatchNode*> pnodes, Vector3 v, double &tScale);

// combination
FdGraph* combineDecomposition(QVector<FdGraph*> decmps, QString baseMid, 
	QMap<QString, QSet<int> >& masterDecmpMap);

// 2D geometry
Geom::Rectangle2 computeAABB2D(QVector<Vector2> &pnts);
bool extendRectangle2D(Geom::Rectangle2& rect, QVector<Vector2> &pnts);

// volume
double volume(QList<Geom::Box> boxes);