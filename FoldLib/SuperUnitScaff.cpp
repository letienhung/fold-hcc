#include "SuperUnitScaff.h"
#include "HUnitScaff.h"
#include "SuperShapeKf.h"
#include "ChainScaff.h"
#include "Numeric.h"

SuperUnitScaff::SuperUnitScaff(HUnitScaff* block, SuperShapeKf* sskf)
: origUnit(block), ssKeyframe(sskf)
{
	// map from master to super master within this block
	// self mapping if no corresponding super master
	QMap<QString, QString> master2Super;

	// clone parts from block
	foreach(Structure::Node* n, origUnit->nodes)
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
	QString base_mid = master2Super[origUnit->baseMaster->mID];
	baseMaster = (PatchNode*)getNode(base_mid);

	// other masters
	foreach(PatchNode* m, origUnit->masters)
		masters << (PatchNode*)getNode(master2Super[m->mID]);
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
	Geom::Rectangle base_rect = origUnit->baseMaster->mPatch;// ***use original base rect
	origUnit->properties.remove(DEBUG_POINTS); // clear debug points
	foreach(PatchNode* top_master, masters)
	{
		// skip base master
		QString top_mid = top_master->mID;
		if (top_mid == base_mid) continue;

		// obstacle parts: in-between and unordered
		QVector<ScaffNode*> obstacleParts;
		// ***no external parts stick in between
		// internal master is allowed to stay in between, because they will be collapsed in order
		obstacleParts << ssKeyframe->getInbetweenExternalParts(origUnit, baseMaster->center(), top_master->center());
		// ***chains under the top master should not stick into other blocks: introducing new order constraints
		// unrelated masters must be external because all masters have relation with the base master
		obstacleParts << ssKeyframe->getUnrelatedExternalMasters(base_mid, top_mid);

		// sample obstacle parts
		QVector<Vector3> samples;
		for (ScaffNode* obsPart : obstacleParts)
			samples << obsPart->sampleBoundabyOfScaffold(100);

		// project to the base rect to get obstacles
		QVector<Vector2> obs;
		for (auto s : samples) obs << base_rect.getProjCoordinates(s);
		obstacles[top_mid] = obs;

		// debug
		origUnit->appendToVectorProperty(DEBUG_POINTS, samples);
	}

	// restore the position of super shape key frame since it is shared
	ssKeyframe->translate(-offset, false);

	// obstacles
	return obstacles;
}
