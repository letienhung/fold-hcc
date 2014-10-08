#include "SuperShapeKf.h"
#include "UnitScaff.h"
#include "GeomUtility.h"

SuperShapeKf::SuperShapeKf(Scaffold* superKeyframe, StringSetMap moc_g)
{
	// clone all nodes
	foreach(Structure::Node* n, superKeyframe->nodes)
		Structure::Graph::addNode(n->clone());

	// super master and their corresponding masters
	QVector<PatchNode*> superMasters;
	QVector<QSet<QString> > childMasters, childMasters_new;
	for(Structure::Node* n : getNodesWithTag(MASTER_TAG)){
		if (n->hasTag(SUPER_MASTER_TAG)) {
			PatchNode* m = (PatchNode*)n;
			superMasters << m;
			childMasters << m->properties[MERGED_MASTERS].value<QSet<QString> >();
		}
	}

	// merge super nodes which share children
	QVector<QSet<int> > superIdxClusters = mergeIsctSets(childMasters, childMasters_new);

	// merge to create new super patches
	for (int i = 0; i < superIdxClusters.size(); i++)
	{
		// merged set indices
		QList<int> superIndices = superIdxClusters[i].toList();

		// pick up the first
		PatchNode* superPatch_new = superMasters[superIndices[0]];
		superPatch_new->mID = QString("MP_%1").arg(i);
		Geom::Rectangle pred_rect_new = superPatch_new->mPatch;

		// merge with others
		QVector<Vector2> pnts2 = pred_rect_new.get2DConners();
		for (int j = 1; j < superIndices.size(); j++)
		{
			int superIdx = superIndices[j];
			Geom::Rectangle2 rect2 = pred_rect_new.get2DRectangle(superMasters[superIdx]->mPatch);
			pnts2 << rect2.getConners();

			// remove other
			this->removeNode(superMasters[superIdx]->mID);
		}
		Geom::Rectangle2 aabb2 = Geom::computeAABB(pnts2);
		superPatch_new->resize(aabb2);

		// store master_super_map
		superPatch_new->properties[MERGED_MASTERS].setValue(childMasters_new[i]);
		foreach(QString mid, childMasters_new[i])
			master2SuperMap[mid] = superPatch_new->mID;
	}

	// update moc_greater
	mocGreater = moc_g;

	// change name of keys
	foreach(QSet<QString> child_mids, childMasters_new)
	{
		QString child = child_mids.toList().front();
		QString key_new = master2SuperMap[child];
		QSet<QString> values_union;
		foreach(QString cmid, child_mids){
			values_union += mocGreater[cmid];
			mocGreater.remove(cmid);
		}
		mocGreater[key_new] = values_union;
	}

	// change name of values
	foreach(QString key, mocGreater.keys())
	{
		QSet<QString> values_new;
		foreach(QString v, mocGreater[key]){
			if (master2SuperMap.contains(v))
				v = master2SuperMap[v]; // change name
			values_new << v;
		}
		mocGreater[key] = values_new;
	}

	// moc_less
	foreach(QString key, mocGreater.keys())
		foreach(QString value, mocGreater[key])
		mocLess[value] << key;
}


// A FdGraph is valid only if all order constraints are met
bool SuperShapeKf::isValid(Vector3 sqzV)
{
	// get all masters: un-merged and super
	QVector<PatchNode*> ms;
	for(Structure::Node* n : getNodesWithTag(MASTER_TAG))
	if (!n->hasTag(MERGED_PART_TAG))	ms << (PatchNode*)n;

	// compute time stamps
	double tScale;
	QMap<QString, double> timeStamps = getTimeStampsNormalized(ms, sqzV, tScale);

	// check validity
	foreach(QString key, mocGreater.keys())
		foreach(QString value, mocGreater[key])
	if (timeStamps[key] < timeStamps[value])
		return false;

	return true;
}

QVector<ScaffNode*> SuperShapeKf::getInbetweenExternalParts(UnitScaff* unit, Vector3 p0, Vector3 p1)
{
	// time line
	Vector3 sqzV = unit->baseMaster->mPatch.Normal;
	Geom::Line timeLine(Vector3(0, 0, 0), sqzV);

	// position on time line
	double t0 = timeLine.getProjTime(p0);
	double t1 = timeLine.getProjTime(p1);
	double epsilon = 0.05 * (t1 - t0);
	TimeInterval m1m2(t0 + epsilon, t1 - epsilon);

	// find parts in between m1 and m2
	QVector<ScaffNode*> inbetweens;
	for(ScaffNode* sn : getScaffNodes())
	{
		// skip parts that has been folded
		if (sn->hasTag(MERGED_PART_TAG)) continue;

		// skip parts in unit
		if (unit->containsNode(sn->mID)) continue;

		// master
		if (sn->hasTag(MASTER_TAG))
		{
			double t = timeLine.getProjTime(sn->center());

			if (m1m2.contains(t)) 	inbetweens << sn;
		}
		else
		// slave		
		{
			int aid = sn->mBox.getAxisId(sqzV);
			Geom::Segment sklt = sn->mBox.getSkeleton(aid);
			double t0 = timeLine.getProjTime(sklt.P0);
			double t1 = timeLine.getProjTime(sklt.P1);
			if (t0 > t1) std::swap(t0, t1);
			TimeInterval ti(t0, t1);

			if (ti.overlaps(m1m2))	inbetweens << sn;
		}
	}

	return inbetweens;
}

QVector<ScaffNode*> SuperShapeKf::getUnrelatedExternalMasters(QString base_mid, QString top_mid)
{
	QVector<ScaffNode*> urMasters;

	// related parts with mid1 and mid2
	QSet<QString> base_moc = mocGreater[base_mid] + mocLess[base_mid];
	QSet<QString> top_moc = mocGreater[top_mid] + mocLess[top_mid];

	// masters unrelated with both
	for(Structure::Node* n : getNodesWithTag(MASTER_TAG))
	{
		// skip folded or virtual masters
		if (n->hasTag(MERGED_PART_TAG) || n->hasTag(EDGE_ROD_TAG)) continue;

		// accept if not related to any
		if (!base_moc.contains(n->mID) && !top_moc.contains(n->mID))
			urMasters << (ScaffNode*)n;
	}

	return urMasters;
}
