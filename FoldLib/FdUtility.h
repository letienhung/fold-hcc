#pragma once

#include "UtilityGlobal.h"
#include "Segment.h"
#include "Box.h"
#include "SectorCylinder.h"

class ScaffNode;
class PatchNode;
class Scaffold;

// typedef
typedef QVector< QVector<ScaffNode*> > FdNodeArray2D;

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
Geom::Segment getDistSegment( ScaffNode* n1, ScaffNode* n2 );
double getDistance( ScaffNode* n1, ScaffNode* n2 );
double getDistance( ScaffNode* n, QVector<ScaffNode*> nset);

// relation among fd nodes
FdNodeArray2D getConnectedGroups( QVector<ScaffNode*> nodes, double disThr );
Geom::Segment detectJointSegment(PatchNode* slave, PatchNode* master);
bool hasIntersection(ScaffNode* slave, PatchNode* master, double thr);

// helpers
QVector<QString> getIds(QVector<ScaffNode*> nodes);
StrArray2D getIds(FdNodeArray2D nodeArray);

// relation with plan
enum PLANE_RELATION{ON_PLANE, POS_PLANE, NEG_PLANE, ISCT_PLANE};
PLANE_RELATION relationWithPlane(ScaffNode* n, Geom::Plane plane, double thr);
bool onPlane( ScaffNode* n, Geom::Plane& plane );

// box fitting
enum BOX_FIT_METHOD{FIT_AABB, FIT_MIN, FIT_PCA};
Geom::Box fitBox(QVector<Vector3>& pnts, BOX_FIT_METHOD method = FIT_PCA);

// bundle nodes
QString getBundleName(const QVector<ScaffNode*>& nodes);
Geom::Box getBundleBox(const QVector<ScaffNode*>& nodes);

QMap<QString, double> getTimeStampsNormalized(QVector<ScaffNode*> nodes, Vector3 v, double &tScale);
QMap<QString, double> getTimeStampsNormalized(QVector<PatchNode*> pnodes, Vector3 v, double &tScale);

// combination
Scaffold* combineScaffolds(QVector<Scaffold*> decmps, QString baseMid, 
	QMap<QString, QSet<int> >& masterDecmpMap);

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
