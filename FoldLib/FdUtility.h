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
typedef QPair<double, double> Interval;
#define	INTERVAL(negDist, posDist) qMakePair<double, double>(negDist, posDist)

// tags
#define BUNDLE_TAG "isBundle"
#define	MASTER_TAG "isMasterPatch"
#define EDGE_ROD_TAG "isEdgeRodNode"
#define DELETED_TAG "hasDeleted"
#define MERGED_PART_TAG "mergedMasterTag"
#define SELECTED_TAG "isSelected"
#define SUPER_PATCH_TAG "isMerged"
#define MERGED_MASTERS "mergedMastersSet"
#define	MASTER_SUPER_MAP "master_prediction_map"
#define DELETE_FOLD_OPTION	"deleteFoldOption"
#define ACTIVE_TAG "isActive"
#define EDGE_ROD_HOST "edgeRodHost"

// propagation
#define FIXED_NODE_TAG "hasFixed"
#define ACTIVE_HINGE_TAG "isActiveHinge"

// debug visual
#define AFS "availFoldingSpace"
#define AFR_CP "AFR_constraint_points"
#define FOLD_REGIONS "filtered_fold_regions"
#define MAXFR "MAXFondingRegion"
#define MINFR "MINFoldingRegion"
#define SHOW_AFS "showAFS"
#define DEBUG_PLANES "debugPlanes"

// statistics
#define NB_SPLIT "nbSplits"
#define NB_CHUNKS "nbChunks"
#define SQZ_DIRECTION "sqzDirection"
#define NB_MASTER "nbMasters"
#define NB_SLAVE "nbSlaves"
#define NB_BLOCK "nbBlocks"
#define NB_HINGES "nbHinges"
#define SHRINKED_AREA "shrinkedArea"
#define FD_TIME "fdTime"
#define SPACE_SAVING "spaceSaving"

#define CONN_THR_RATIO "connThrRatio"
#define CONSTRAIN_AABB_SCALE "constrainAabbScale"
#define COST_WEIGHT "costWeight"

// Qt meta type
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(QVector<Vector3>)
Q_DECLARE_METATYPE(Geom::SectorCylinder)
Q_DECLARE_METATYPE(QVector<Geom::SectorCylinder>)
Q_DECLARE_METATYPE(Geom::Rectangle)
Q_DECLARE_METATYPE(QVector<Geom::Rectangle>)
Q_DECLARE_METATYPE(QVector<Geom::Segment>)
Q_DECLARE_METATYPE(QVector<Geom::Box>)
typedef QMap<QString, QString> StringStringMap;
Q_DECLARE_METATYPE(StringStringMap)
typedef QMap<QString, QSet<QString> > StringSetMap;
Q_DECLARE_METATYPE(StringSetMap)
Q_DECLARE_METATYPE(QSet<QString>)
Q_DECLARE_METATYPE(QVector<Geom::Plane>)

// distance between fd nodes
Geom::Segment getDistSegment( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n, QVector<FdNode*> nset);

// relation among fd nodes
FdNodeArray2D getConnectedGroups( QVector<FdNode*> nodes, double disThr );
Geom::Segment detectJointSegment(PatchNode* slave, PatchNode* master);
bool hasIntersection(FdNode* slave, PatchNode* master, double thr);

// helpers
QVector<QString> getIds(QVector<FdNode*> nodes);
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
bool overlap(Interval itv1, Interval itv2);
bool within(double t, Interval itv);
bool passed(double t, Interval itv);
double getLocalTime(double globalT, Interval itv);

// masters
QVector<PatchNode*> getAllMasters(FdGraph* scaffold);
QVector<QString> getAllMasterIds(FdGraph* scaffold);
int nbMasters(FdGraph* scaffold);
QMap<QString, double> getTimeStampsNormalized(QVector<FdNode*> nodes, Vector3 v, double &tScale);
QMap<QString, double> getTimeStampsNormalized(QVector<PatchNode*> pnodes, Vector3 v, double &tScale);

// combination
FdGraph* combineFdGraphs(QVector<FdGraph*> decmps, QString baseMid, 
	QMap<QString, QSet<int> >& masterDecmpMap);

// 2D geometry
Geom::Rectangle2 computeAABB2D(QVector<Vector2> &pnts);
bool extendRectangle2D(Geom::Rectangle2& rect, QVector<Vector2> &pnts);

// volume
double volume(QList<Geom::Box> boxes);

QVector<double> findRoots(QVector<double>& coeff);
QVector<double> findRoots(double a, double b, double c);
QVector<double> findRoots(double a, double b, double c, double d, double e);

// cluster intersecting sets
// return clustered set idx; merged sets are stored in \p merged_sets
template <class T>
QVector<QSet<int> > mergeIsctSets(QVector<QSet<T> > &sets, QVector<QSet<T> > &merged_sets)
{
	QMap<int, QSet<T> > merged_sets_map;
	QMap<int, QSet<int> > merged_idx_map;
	int count = 0;
	for (int sid = 0; sid < sets.size(); sid++)
	{
		QSet<T> set = sets[sid];
		QSet<int> setIdx;
		setIdx << sid;
		foreach (int key, merged_sets_map.keys())
		{
			QSet<T> isct = merged_sets_map[key] & set;
			if (!isct.isEmpty()) 
			{
				// merge and remove old cluster
				set += merged_sets_map[key];
				setIdx += merged_idx_map[key];
				merged_sets_map.remove(key);
				merged_idx_map.remove(key);
			}
		}

		// create a new merged cluster
		merged_sets_map[count] = set;
		merged_idx_map[count] = setIdx;
		count++;
	}

	// result
	merged_sets = merged_sets_map.values().toVector();
	return merged_idx_map.values().toVector();
}

void print(Vector3 v);
void print(Geom::Box box);
