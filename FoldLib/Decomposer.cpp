#include "Decomposer.h"

#include "DecScaff.h"
#include "TUnitScaff.h"
#include "HUnitScaff.h"
#include "ParSingleton.h"

Decomposer::Decomposer(DecScaff* ss)
{
	scfd = ss;
}

void Decomposer::execute()
{
	createMasters();
	createSlaves();
	clusterSlaves();
}

ScaffNodeArray2D Decomposer::getPerpConnGroups()
{
	// squeezing direction
	Vector3 sqzV = ParSingleton::instance()->sqzV;
	Geom::AABB aabb = scfd->computeAABB();
	Geom::Box box = aabb.box();
	int sqzAId = aabb.box().getAxisId(sqzV);

	// threshold
	double perpThr = 0.1;
	double connThr = scfd->getConnectThr();

	// ==STEP 1==: nodes perp to squeezing direction
	QVector<PatchNode*> perpNodes;
	for (ScaffNode* n : scfd->getScaffNodes())
	{
		// perp node: rod or patch
		if (n->isPerpTo(sqzV, perpThr))
		{
			PatchNode* pn;
			if (n->mType == ScaffNode::ROD)
				pn = scfd->changeRodToPatch((RodNode*)n, sqzV);
			else
				pn = (PatchNode*)n;

			perpNodes << pn;
		}
		else if (n->mType == ScaffNode::PATCH)
		{
			// edge rods perp to sqzV
			PatchNode* pn = (PatchNode*)n;
			for (RodNode* rn : pn->getEdgeRodNodes())
			{
				// add virtual rod nodes 
				if (rn->isPerpTo(sqzV, perpThr))
				{
					PatchNode* prn = new PatchNode(rn, sqzV);
					prn->addTag(EDGE_VIRTUAL_TAG);
					scfd->Structure::Graph::addNode(prn);

					perpNodes << prn;
				}

				// garbage collection
				delete rn;
			}
		}
	}

	// ==STEP 2==: group perp nodes
	// perp positions
	Geom::Box shapeBox = scfd->computeAABB().box();
	Geom::Segment skeleton = shapeBox.getSkeleton(sqzAId);
	double perpGroupThr = connThr / skeleton.length();
	QMultiMap<double, ScaffNode*> posNodeMap;
	for (ScaffNode* n : perpNodes){
		posNodeMap.insert(skeleton.getProjCoordinates(n->mBox.Center), n);
	}
	ScaffNodeArray2D perpGroups;
	QList<double> perpPos = posNodeMap.uniqueKeys();
	double pos0 = perpPos.front();
	perpGroups << posNodeMap.values(pos0).toVector();
	for (int i = 1; i < perpPos.size(); i++)
	{
		QVector<ScaffNode*> currNodes = posNodeMap.values(perpPos[i]).toVector();
		if (fabs(perpPos[i] - perpPos[i - 1]) < perpGroupThr)
			perpGroups.last() += currNodes;	// append to current group
		else perpGroups << currNodes;		// create new group
	}

	// ==STEP 3==: perp & connected groups
	ScaffNodeArray2D perpConnGroups;
	perpConnGroups << perpGroups.front(); // ground
	for (int i = 1; i < perpGroups.size(); i++){
		for (QVector<ScaffNode*> connGroup : getConnectedGroups(perpGroups[i], connThr))
			perpConnGroups << connGroup;
	}

	return perpConnGroups;
}

void Decomposer::createMasters()
{
	ScaffNodeArray2D perpConnGroups = getPerpConnGroups();
	Vector3 sqzV = ParSingleton::instance()->sqzV;

	// merge connected groups into patches
	for(auto pcGroup : perpConnGroups)
	{
		// single node
		if (pcGroup.size() == 1)
		{
			scfd->masters << (PatchNode*)pcGroup.front();
		}
		// multiple nodes
		else
		{
			// check if all nodes in the pcGroup is virtual
			bool allVirtual = true;
			for (ScaffNode* pcNode : pcGroup){
				if (!pcNode->hasTag(EDGE_VIRTUAL_TAG))
				{
					allVirtual = false;
					break;
				}
			}

			// create bundle master
			if (!allVirtual)
				scfd->masters << (PatchNode*)scfd->wrapAsBundleNode(getIds(pcGroup), sqzV);
			// treat edge virtual nodes individually
			else for (ScaffNode* pcNode : pcGroup)
					scfd->masters << (PatchNode*)pcNode;
		}
	}

	// normal and tag
	for (PatchNode* m : scfd->masters)
	{
		// consistent normal with sqzV
		if (dot(m->mPatch.Normal, sqzV) < 0)
			m->mPatch.flipNormal();

		// tag
		m->addTag(MASTER_TAG);
	}
}

void Decomposer::updateSlaves()
{
	// collect non-master parts as slaves
	scfd->slaves.clear();
	for(ScaffNode* n : scfd->getScaffNodes())
	if (!n->hasTag(MASTER_TAG))	scfd->slaves << n;
}

void Decomposer::updateSlaveMasterRelation()
{
	// initial
	scfd->slave2master.clear();
	scfd->slave2master.resize(scfd->slaves.size());

	// find two masters attaching to a slave
	double adjacentThr = scfd->getConnectThr();
	for (int i = 0; i < scfd->slaves.size(); i++)
	{
		// find adjacent master(s)
		ScaffNode* slave = scfd->slaves[i];
		for (int j = 0; j < scfd->masters.size(); j++)
		{
			PatchNode* master = scfd->masters[j];

			// skip if not attached
			if (getDistance(slave, master) > adjacentThr) continue;


			// virtual edge rod master
			if (master->hasTag(EDGE_VIRTUAL_TAG))
			{
				// can only attach to its host slave
				QString masterOrig = master->properties[EDGE_ROD_ORIG].value<QString>();
				QString slaveOrig = slave->mID;
				if (slave->properties.contains(SPLIT_ORIG))
					slaveOrig = slave->properties[SPLIT_ORIG].value<QString>();
				if (masterOrig == slaveOrig) scfd->slave2master[i] << j;
			}
			// real master
			else
			{
				scfd->slave2master[i] << j;
			}
		}
	}
}

void Decomposer::createSlaves()
{
	// split slave parts by master patches
	double connThr = scfd->getConnectThr();
	for (PatchNode* master : scfd->masters) 
	{
		// skip single edge virtual
		if (master->hasTag(EDGE_VIRTUAL_TAG)) continue;

		// split
		for (ScaffNode* n : scfd->getScaffNodes())
		{
			// skip masters
			if (n->hasTag(MASTER_TAG)) continue;

			// split connected slaves
			if (hasIntersection(n, master, connThr))
				scfd->split(n->mID, master->mPatch.getPlane());
		}
	}

	// slaves and slave-master relation
	updateSlaves();
	updateSlaveMasterRelation();

	// remove unbounded slaves: unlikely to happen
	int nDeleted = 0;
	for (int i = 0; i < scfd->slaves.size(); i++)
	{
		if (scfd->slave2master[i].isEmpty())
		{
			scfd->slaves.remove(i - nDeleted);
			nDeleted++;
		}
	}
}

void Decomposer::clusterSlaves()
{
	// clear
	scfd->slaveClusters.clear();
	QVector<bool> slaveVisited(scfd->slaves.size(), false);

	// build master(V)-slave(E) graph
	Structure::Graph* ms_graph = new Structure::Graph();
	for (int i = 0; i <scfd->masters.size(); i++)
	{
		Structure::Node* n = new Structure::Node(QString::number(i));
		ms_graph->addNode(n); // masters are nodes
	}
	for (int i = 0; i < scfd->slaves.size(); i++)
	{
		// H-slave are links
		QList<int> midPair = scfd->slave2master[i].toList();
		if (midPair.size() != 2) continue; // T-slave
		QString nj = QString::number(midPair.front());
		QString nk = QString::number(midPair.last());
		if (ms_graph->getLink(nj, nk) == nullptr)
			ms_graph->addLink(nj, nk);
	}

	// enumerate all chordless cycles
	QVector<QVector<QString> > cycle_base = ms_graph->findCycleBase();
	delete ms_graph;

	// collect slaves along each cycle
	QVector<QSet<int> > cycle_slaves;
	for (QVector<QString> cycle : cycle_base)
	{
		QSet<int> cs;
		for (int i = 0; i < cycle.size(); i++)
		{
			// two master ids
			QSet<int> midPair;
			midPair << cycle[i].toInt() << cycle[(i + 1) % cycle.size()].toInt();

			// find all siblings: slaves sharing the same pair of masters
			for (int sid = 0; sid < scfd->slaves.size(); sid++){
				if (midPair == scfd->slave2master[sid])
					cs << sid;
			}
		}
		cycle_slaves << cs;
	}

	// merge cycles who share slaves
	mergeIsctSets(cycle_slaves, scfd->slaveClusters);
	for (auto cs : scfd->slaveClusters)
	for(int sid : cs) slaveVisited[sid] = true;

	// edges that are not covered by cycles form individual clusters
	for (int i = 0; i < scfd->slaves.size(); i++)
	{
		// skip covered slaves
		if (slaveVisited[i]) continue;

		// create new cluster
		QSet<int> cluster;
		cluster << i;
		slaveVisited[i] = true;

		// find siblings
		for (int sid = 0; sid < scfd->slaves.size(); sid++){
			if (!slaveVisited[sid] && scfd->slave2master[i] == scfd->slave2master[sid])
			{
				cluster << sid;
				slaveVisited[sid] = true;
			}
		}
		scfd->slaveClusters << cluster;
	}
}