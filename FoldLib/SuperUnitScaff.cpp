#include "SuperUnitScaff.h"
#include "HUnitScaff.h"
#include "ShapeSuperKeyframe.h"
#include "ChainScaff.h"
#include "Numeric.h"

SuperUnitScaff::SuperUnitScaff(HUnitScaff* block, ShapeSuperKeyframe* sskf)
: origBlock(block), ssKeyframe(sskf)
{
	// map from master to super master within this block
	// self mapping if no corresponding super master
	QMap<QString, QString> master2Super;

	// clone parts from block
	foreach(Structure::Node* n, origBlock->nodes)
	{
		Structure::Node* clone_n;

		// super patch (master)
		if (ssKeyframe->master2SuperMap.contains(n->mID))
		{
			Structure::Node* super_n = ssKeyframe->getNode(ssKeyframe->master2SuperMap[n->mID]);
			clone_n = super_n->clone();

			// copy mapping
			master2Super[n->mID] = ssKeyframe->master2SuperMap[n->mID];
		}
		// regular node
		else
		{
			clone_n = n->clone();

			// self mapping
			master2Super[n->mID] = n->mID;
		}

		// add to super block
		Structure::Graph::addNode(clone_n);
	}

	// the base master
	QString base_mid = master2Super[origBlock->baseMaster->mID];
	baseMaster = (PatchNode*)getNode(base_mid);

	// other masters
	foreach(PatchNode* m, origBlock->masters)
		masters << (PatchNode*)getNode(master2Super[m->mID]);
}

QVector<QString> SuperUnitScaff::getUnrelatedExternalMasters(QString base_mid, QString top_mid)
{
	QVector<QString> urMasters;

	// retrieve master order constraints
	StringSetMap moc_greater = ssKeyframe->mocGreater;
	StringSetMap moc_less = ssKeyframe->mocLess;

	// related parts with mid1 and mid2
	QSet<QString> base_moc = moc_greater[base_mid] + moc_less[base_mid];
	QSet<QString> top_moc = moc_greater[top_mid] + moc_less[top_mid];

	// masters unrelated with both
	foreach(PatchNode* m, getAllMasters(ssKeyframe))
	{
		// skip folded or virtual masters
		if (m->hasTag(MERGED_PART_TAG) || m->hasTag(EDGE_ROD_TAG)) continue;

		// accept if not related to any
		if (!base_moc.contains(m->mID) && !top_moc.contains(m->mID))
			urMasters << m->mID;
	}

	return urMasters;
}

QMap< QString, QVector<Vector2> > SuperUnitScaff::computeObstacles()
{
	QMap< QString, QVector<Vector2> > obstacles;

	// align super shape key frame with this super block
	Vector3 pos_block = baseMaster->center();
	ScaffNode* fnode = (ScaffNode*)ssKeyframe->getNode(baseMaster->mID);
	if (!fnode) return obstacles;
	Vector3 pos_keyframe = fnode->center();
	Vector3 offset = pos_block - pos_keyframe;
	ssKeyframe->translate(offset, false);

	// obstacles for each top master
	QString base_mid = baseMaster->mID;
	Geom::Rectangle base_rect = origBlock->baseMaster->mPatch;// ***use original base rect
	origBlock->properties.remove(DEBUG_POINTS); // clear debug points
	foreach(PatchNode* top_master, masters)
	{
		// skip base master
		QString top_mid = top_master->mID;
		if (top_mid == base_mid) continue;

		// obstacle parts: in-between and unordered
		QVector<QString> obstacleParts;
		// ***no external parts stick in between
		// internal master is allowed to stay in between, because they will be collapsed in order
		obstacleParts << origBlock->getInbetweenExternalParts(baseMaster->center(), top_master->center(), ssKeyframe);
		// ***chains under the top master should not stick into other blocks: introducing new order constraints
		// unrelated masters must be external because all masters have relation with the base master
		obstacleParts << getUnrelatedExternalMasters(base_mid, top_mid);

		// sample obstacle parts
		QVector<Vector3> samples;
		for (auto nid : obstacleParts)
			samples << ssKeyframe->getFdNode(nid)->sampleBoundabyOfScaffold(100);

		// project to the base rect to get obstacles
		QVector<Vector2> obs;
		for (auto s : samples) obs << base_rect.getProjCoordinates(s);
		obstacles[top_mid] = obs;

		// debug
		origBlock->appendToVectorProperty(DEBUG_POINTS, samples);
	}

	// restore the position of super shape key frame since it is shared
	ssKeyframe->translate(-offset, false);

	// obstacles
	return obstacles;
}
