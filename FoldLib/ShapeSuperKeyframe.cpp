#include "ShapeSuperKeyframe.h"

ShapeSuperKeyframe::ShapeSuperKeyframe(FdGraph* superKeyframe, StringSetMap moc_g)
{
	// clone all nodes
	foreach(Structure::Node* n, superKeyframe->nodes)
		Structure::Graph::addNode(n);

	// super master and their corresponding masters
	QVector<PatchNode*> superMasters;
	QVector<QSet<QString> > childMasters, childMasters_new;
	foreach(PatchNode* m, getAllMasters(this)){
		if (m->hasTag(SUPER_PATCH_TAG)) {
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
		Geom::Rectangle2 aabb2 = computeAABB2D(pnts2);
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
bool ShapeSuperKeyframe::isValid(Vector3 sqzV)
{
	// get all masters: un-merged and super
	QVector<PatchNode*> ms;
	foreach(PatchNode* m, getAllMasters(this))
		if (!m->hasTag(MERGED_PART_TAG))	ms << m;

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