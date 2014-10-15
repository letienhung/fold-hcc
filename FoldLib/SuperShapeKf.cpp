#include "SuperShapeKf.h"
#include "UnitScaff.h"
#include "GeomUtility.h"

// superKeyframe contains all super masters generated by folded units
SuperShapeKf::SuperShapeKf(Scaffold* superKeyframe, StringSetMap moc_g, Vector3 sV)
: sqzV(sV)
{
	// clone all nodes
	for (Structure::Node* n : superKeyframe->nodes)
		Structure::Graph::addNode(n->clone());

	// super masters and their enclosed regular masters
	QVector<SuperPatchNode*> superMasters;
	QVector<QSet<QString> > enclosedMasters, enclosedMastersNew;
	for(Structure::Node* n : getNodesWithTag(MASTER_TAG)){
		if (n->hasTag(SUPER_PATCH_TAG)) {
			SuperPatchNode* sp = (SuperPatchNode*)n;
			superMasters << sp;
			enclosedMasters << sp->enclosedPatches;
		}
	}

	// merge super nodes which share children
	QVector<QSet<int> > superIdxClusters = mergeIsctSets(enclosedMasters, enclosedMastersNew);

	// merge to create new super patches
	for (int i = 0; i < superIdxClusters.size(); i++)
	{
		// merged set indices
		QList<int> superIndices = superIdxClusters[i].toList();

		// pick up the first
		SuperPatchNode* superPatchNew = superMasters[superIndices[0]];
		superPatchNew->mID = QString("MP_%1").arg(i);
		Geom::Rectangle base_rect = superPatchNew->mPatch;

		// merge with others
		QVector<Vector2> pnts2 = base_rect.get2DConners();
		for (int superIdx : superIndices)
		{
			Geom::Rectangle2 rect2 = base_rect.get2DRectangle(superMasters[superIdx]->mPatch);
			pnts2 << rect2.getConners();

			// remove other
			removeNode(superMasters[superIdx]->mID);
		}
		Geom::Rectangle2 aabb2 = Geom::computeAABB(pnts2);
		superPatchNew->resize(aabb2);

		// update master_super_map
		superPatchNew->enclosedPatches = enclosedMastersNew[i];
		for (QString mid : enclosedMastersNew[i])
			master2SuperMap[mid] = superPatchNew->mID;
	}

	// ***update moc_greater
	// change name of keys
	mocGreater = moc_g;
	for (QSet<QString> child_mids : enclosedMastersNew)
	{
		QString child = child_mids.toList().front();
		QString key_new = master2SuperMap[child];
		QSet<QString> values_union;
		for (QString cmid : child_mids){
			values_union += mocGreater[cmid];
			mocGreater.remove(cmid);
		}
		mocGreater[key_new] = values_union;
	}

	// change name of values
	for (QString key : mocGreater.keys())
	{
		QSet<QString> values_new;
		for (QString v : mocGreater[key]){
			if (master2SuperMap.contains(v))
				v = master2SuperMap[v]; // change name
			values_new << v;
		}
		mocGreater[key] = values_new;
	}

	// moc_less
	for (QString key : mocGreater.keys())
	for (QString value : mocGreater[key])
		mocLess[value] << key;
}


// A scaffold is valid only if all order constraints are met
bool SuperShapeKf::isValid()
{
	// get all masters: un-merged and super
	QVector<PatchNode*> ms;
	for(Structure::Node* n : getNodesWithTag(MASTER_TAG))
	if (!n->hasTag(MERGED_PART_TAG))	ms << (PatchNode*)n;

	// compute time stamps
	double tScale;
	QMap<QString, double> timeStamps = getTimeStampsNormalized(ms, sqzV, tScale);

	// check validity
	for (QString key : mocGreater.keys())
	for (QString value : mocGreater[key])
	if (timeStamps[key] < timeStamps[value])
		return false;

	return true;
}

QVector<ScaffNode*> SuperShapeKf::getInbetweenParts(QString base_mid, QString top_mid)
{
	// time line
	Geom::Line timeLine(Vector3(0, 0, 0), sqzV);

	// position on time line
	Vector3 p0 = ((ScaffNode*)getNode(base_mid))->center();
	Vector3 p1 = ((ScaffNode*)getNode(top_mid))->center();
	double t0 = timeLine.getProjTime(p0);
	double t1 = timeLine.getProjTime(p1);
	double epsilon = 0.05 * (t1 - t0);
	TimeInterval ti(t0 + epsilon, t1 - epsilon);

	// find parts in between m1 and m2
	QVector<ScaffNode*> inbetweens;
	for(ScaffNode* sn : getScaffNodes())
	{
		// skip parts that has been folded
		if (sn->hasTag(MERGED_PART_TAG)) continue;

		// master
		if (sn->hasTag(MASTER_TAG))
		{
			double t = timeLine.getProjTime(sn->center());

			if (ti.contains(t)) 	inbetweens << sn;
		}
		else
		// slave		
		{
			int aid = sn->mBox.getAxisId(sqzV);
			Geom::Segment sklt = sn->mBox.getSkeleton(aid);
			double t0 = timeLine.getProjTime(sklt.P0);
			double t1 = timeLine.getProjTime(sklt.P1);
			if (t0 > t1) std::swap(t0, t1);
			TimeInterval sTi(t0, t1);

			if (ti.overlaps(sTi))	inbetweens << sn;
		}
	}

	return inbetweens;
}

QVector<ScaffNode*> SuperShapeKf::getUnrelatedMasters(QString regular_mid)
{
	QVector<ScaffNode*> urMasters;

	// do nothing for virtual master
	if (getNode(regular_mid)->hasTag(EDGE_ROD_TAG)) return urMasters;

	// the super master
	QString super_mid = regular_mid;
	if (master2SuperMap.contains(regular_mid))
		super_mid = master2SuperMap[regular_mid];

	// masters unrelated to super master
	QSet<QString> relatedMids = mocGreater[super_mid] + mocLess[super_mid];
	for(Structure::Node* node : getNodesWithTag(MASTER_TAG))
	{
		// skip folded or virtual masters
		if (node->hasTag(MERGED_PART_TAG) || node->hasTag(EDGE_ROD_TAG)) continue;

		// accept if not related
		if (!relatedMids.contains(node->mID))
			urMasters << (ScaffNode*)node;
	}

	return urMasters;
}