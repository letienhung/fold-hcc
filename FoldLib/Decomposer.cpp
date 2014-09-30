#include "Decomposer.h"

#include "ShapeScaffold.h"
#include "TUnitScaffold.h"
#include "HUnitScaffold.h"

Decomposer::Decomposer(ShapeScaffold* ss)
{
	scfd = ss;
}

void Decomposer::execute()
{
	createMasters();
	createSlaves();
	clusterSlaves();
}

FdNodeArray2D Decomposer::getPerpConnGroups()
{
	// squeezing direction
	Geom::AABB aabb = scfd->computeAABB();
	Geom::Box box = aabb.box();
	int sqzAId = aabb.box().getAxisId(scfd->sqzV);

	// threshold
	double perpThr = 0.1;
	double connThr = scfd->getConnectivityThr();

	// ==STEP 1==: nodes perp to squeezing direction
	QVector<ScaffoldNode*> perpNodes;
	foreach(ScaffoldNode* n, scfd->getScfdNodes())
	{
		// perp node
		if (n->isPerpTo(scfd->sqzV, perpThr))
		{
			perpNodes << n;
		}
		// virtual rod nodes from patch edges
		else if (n->mType == ScaffoldNode::PATCH)
		{
			PatchNode* pn = (PatchNode*)n;
			foreach(RodNode* rn, pn->getEdgeRodNodes())
			{
				// add virtual rod nodes 
				if (rn->isPerpTo(scfd->sqzV, perpThr))
				{
					scfd->Structure::Graph::addNode(rn);
					perpNodes << rn;
					rn->addTag(EDGE_ROD_TAG);
				}
				// delete if not need
				else
				{
					delete rn;
				}
			}
		}
	}

	// ==STEP 2==: group perp nodes
	// perp positions
	Geom::Box shapeBox = scfd->computeAABB().box();
	Geom::Segment skeleton = shapeBox.getSkeleton(sqzAId);
	double perpGroupThr = connThr / skeleton.length();
	QMultiMap<double, ScaffoldNode*> posNodeMap;
	foreach(ScaffoldNode* n, perpNodes){
		posNodeMap.insert(skeleton.getProjCoordinates(n->mBox.Center), n);
	}
	FdNodeArray2D perpGroups;
	QList<double> perpPos = posNodeMap.uniqueKeys();
	double pos0 = perpPos.front();
	perpGroups << posNodeMap.values(pos0).toVector();
	for (int i = 1; i < perpPos.size(); i++)
	{
		QVector<ScaffoldNode*> currNodes = posNodeMap.values(perpPos[i]).toVector();
		if (fabs(perpPos[i] - perpPos[i - 1]) < perpGroupThr)
			perpGroups.last() += currNodes;	// append to current group
		else perpGroups << currNodes;		// create new group
	}

	// ==STEP 3==: perp & connected groups
	FdNodeArray2D perpConnGroups;
	perpConnGroups << perpGroups.front(); // ground
	for (int i = 1; i < perpGroups.size(); i++){
		foreach(QVector<ScaffoldNode*> connGroup, getConnectedGroups(perpGroups[i], connThr))
			perpConnGroups << connGroup;
	}

	return perpConnGroups;
}

void Decomposer::createMasters()
{
	FdNodeArray2D perpConnGroups = getPerpConnGroups();

	// merge connected groups into patches
	for(auto pcGroup : perpConnGroups)
	{
		// single node
		if (pcGroup.size() == 1)
		{
			ScaffoldNode* n = pcGroup.front();
			if (n->mType == ScaffoldNode::PATCH)	scfd->masters << (PatchNode*)n;
			else scfd->masters << scfd->changeRodToPatch((RodNode*)n, scfd->sqzV);
		}
		// multiple nodes
		else
		{
			// check if all nodes in the pcGroup is virtual
			bool allVirtual = true;
			for (ScaffoldNode* pcNode : pcGroup){
				if (!pcNode->hasTag(EDGE_ROD_TAG))
				{
					allVirtual = false;
					break;
				}
			}

			// create bundle master
			if (!allVirtual)
				scfd->masters << (PatchNode*)scfd->wrapAsBundleNode(getIds(pcGroup), scfd->sqzV);
			// each edge rod is converted to a patch master
			else for (ScaffoldNode* pcNode : pcGroup)
				scfd->masters << scfd->changeRodToPatch((RodNode*)pcNode, scfd->sqzV);
		}
	}

	// normal and tag
	for (PatchNode* m : scfd->masters)
	{
		// consistent normal with sqzV
		if (dot(m->mPatch.Normal, scfd->sqzV) < 0)
			m->mPatch.flipNormal();

		// tag
		m->addTag(MASTER_TAG);
	}
}

void Decomposer::updateSlaves()
{
	// collect non-master parts as slaves
	scfd->slaves.clear();
	for(ScaffoldNode* n : scfd->getScfdNodes())
	if (!n->hasTag(MASTER_TAG))	scfd->slaves << n;
}

void Decomposer::updateSlaveMasterRelation()
{
	// initial
	scfd->slave2master.clear();
	scfd->slave2master.resize(scfd->slaves.size());

	// find two masters attaching to a slave
	double adjacentThr = scfd->getConnectivityThr();
	for (int i = 0; i < scfd->slaves.size(); i++)
	{
		// find adjacent master(s)
		ScaffoldNode* slave = scfd->slaves[i];
		for (int j = 0; j < scfd->masters.size(); j++)
		{
			PatchNode* master = scfd->masters[j];

			// skip if not attached
			if (getDistance(slave, master) > adjacentThr) continue;


			// virtual edge rod master
			if (master->hasTag(EDGE_ROD_TAG))
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
	double connThr = scfd->getConnectivityThr();
	for (PatchNode* master : scfd->masters) {
		for (ScaffoldNode* n : scfd->getScfdNodes())
		{
			if (n->hasTag(MASTER_TAG)) continue;
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

	// deform each slave slightly so that it attaches to masters perfectly
	for (int i = 0; i < scfd->slaves.size(); i++)
	for (int mid : scfd->slave2master[i])
		scfd->slaves[i]->deformToAttach(scfd->masters[mid]->mPatch.getPlane());
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
		if (ms_graph->getLink(nj, nk) == NULL)
			ms_graph->addLink(nj, nk);
	}

	// enumerate all chordless cycles
	QVector<QVector<QString> > cycle_base = ms_graph->findCycleBase();
	delete ms_graph;

	// collect slaves along each cycle
	QVector<QSet<int> > cycle_slaves;
	foreach(QVector<QString> cycle, cycle_base)
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

