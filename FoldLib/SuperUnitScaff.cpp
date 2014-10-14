#include "SuperUnitScaff.h"
#include "UnitScaff.h"
#include "SuperShapeKf.h"
#include "ChainScaff.h"
#include "Numeric.h"

SuperUnitScaff::SuperUnitScaff(UnitScaff* unit, SuperShapeKf* sskf)
: origUnit(unit), ssKeyframe(sskf)
{
	// map from master to super master within this block
	// self mapping if no corresponding super master
	QMap<QString, QString> master2Super;

	// clone parts from original unit
	for (Structure::Node* node : origUnit->nodes)
	{
		Structure::Node* cloneNode;

		// super patch (master)
		if (ssKeyframe->master2SuperMap.contains(node->mID))
		{
			Structure::Node* superNode = ssKeyframe->getNode(ssKeyframe->master2SuperMap[node->mID]);
			cloneNode = superNode->clone();

			// copy mapping
			master2Super[node->mID] = ssKeyframe->master2SuperMap[node->mID];
		}
		// regular node
		else
		{
			cloneNode = node->clone();

			// self mapping
			master2Super[node->mID] = node->mID;
		}

		// add to super unit
		Structure::Graph::addNode(cloneNode);
	}

	// the base master
	QString base_mid = master2Super[origUnit->baseMaster->mID];
	baseMaster = (PatchNode*)getNode(base_mid);

	// all masters
	for (PatchNode* m : origUnit->masters)
		masters << (PatchNode*)getNode(master2Super[m->mID]);
}