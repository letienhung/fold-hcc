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
#define	IS_MASTER "isMasterPatch"
#define IS_EDGE_ROD "isEdgeRodNode"
#define IS_HBLOCK_FOLD_ENTITY "isHBlockFoldEntity"
#define IS_HBLOCK_FOLD_OPTION "isHBlockFoldOption"
#define SELECTED_FOLD_OPTION "selectedFoldOption"
#define DELETED_TAG "hasdeleted"

// Qt meta type
Q_DECLARE_METATYPE(Geom::SectorCylinder)
Q_DECLARE_METATYPE(Geom::Rectangle2)
Q_DECLARE_METATYPE(QVector<Geom::Segment>)

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
double getLocalTime(double globalT, TimeInterval itv);

// masters
QVector<PatchNode*> getAllMasters(FdGraph* scaffold);
FdGraph* combineDecomposition(QVector<FdGraph*> decmps, QString baseMid, 
	QMap<QString, QSet<int> >& masterDecmpMap);