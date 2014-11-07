#include "DecScaff.h"
#include "PatchNode.h"
#include <QFileInfo>
#include "FdUtility.h"
#include "SectorCylinder.h"
#include "ChainScaff.h"
#include "IntrRect2Rect2.h"
#include "TUnitScaff.h"
#include "HUnitScaff.h"
#include "ZUnitScaff.h"
#include "Decomposer.h"
#include "ParSingleton.h"

DecScaff::DecScaff(QString id, Scaffold* scaffold)
	: Scaffold(*scaffold), baseMaster(nullptr) // clone the FdGraph
{
	path = QFileInfo(path).absolutePath();
	mID = id;

	// decompose into units
	createUnits();

	// unit weights
	computeUnitImportance();

	// master order constraints
	// to-do: order constraints among all parts
	computeMasterOrderConstraints();

	// time scale
	double totalDuration = 0;
	for (UnitScaff* b : units) totalDuration += b->getNbTopMasters();
	timeScale = 1.0 / totalDuration;

	// selection
	selUnitIdx = -1;
	keyframeIdx = -1;
}

DecScaff::~DecScaff()
{
	for (UnitScaff* l : units)	delete l;
}


bool DecScaff::areParallel(QVector<ScaffNode*>& ns)
{
	// all patch nodes
	QVector<PatchNode*> pns;
	for (ScaffNode* n : ns)
		if (n->mType == ScaffNode::PATCH)
			pns << (PatchNode*)n;

	// no patches
	if (pns.isEmpty()) return true;

	// consistency of patch normals
	bool parallel = true;
	Vector3 N0 = pns[0]->mPatch.Normal;
	for (int i = 0; i < pns.size(); i++)
	{
		Vector3 Ni = pns[i]->mPatch.Normal;
		double dotProd = dot(N0, Ni);
		if (fabs(dotProd) < 0.95)
		{
			parallel = false;
			break;
		}
	}

	return parallel;
}

UnitScaff* DecScaff::createUnit(QSet<int> sCluster)
{
	// slaves and master pairs
	QVector<ScaffNode*> ss;
	QVector< QVector<QString> > mPairs;
	QSet<int> mids; // unique master indices
	for(int sidx : sCluster)
	{
		ss << slaves[sidx]; // slaves

		QVector<QString> mp;
		for (int mid : slave2master[sidx]){
			mids << mid;
			mp << masters[mid]->mID;
		}

		mPairs << mp; // master pairs
	}

	// masters
	QVector<PatchNode*> ms;
	for(int mid : mids)	ms << masters[mid];

	// map from master to unit
	int unitIdx = units.size();
	for (PatchNode* m : ms) masterUnitMap[m->mID] << unitIdx;

	// create
	UnitScaff* unit;
	QString id = QString::number(unitIdx);
	if (ms.size() == 2					// two masters
		&& ss.size() == 1				// one slave
		&& (ms[0]->hasTag(EDGE_VIRTUAL_TAG)
		|| ms[1]->hasTag(EDGE_VIRTUAL_TAG)))// one master is edge rod
	{
		unit = new TUnitScaff("TB_" + id, ms, ss, mPairs);
	}
	else if (ms.size() == 2				// two masters
		&& !ms[0]->hasTag(EDGE_VIRTUAL_TAG)
		&& !ms[1]->hasTag(EDGE_VIRTUAL_TAG) // both masters are real
		&& areParallel(ss))				// slaves are in parallel
	{
		unit = new ZUnitScaff("ZB_" + id, ms, ss, mPairs);
	}
	else
	{
		unit = new HUnitScaff("HB_" + id, ms, ss, mPairs);
	}

	return unit;
}

void DecScaff::createUnits()
{
	// clear units
	for (auto u : units) delete u;
	units.clear();
	masterUnitMap.clear();

	// decompose
	Decomposer decmp(this);
	decmp.execute();

	// each slave cluster forms a block
	for (auto sc : slaveClusters)
		units << createUnit(sc);
}

void DecScaff::computeMasterOrderConstraints()
{
	// time stamps: bottom-up
	double tScale;
	Vector3 sqzV = ParSingleton::instance()->sqzV;
	QMap<QString, double> masterTimeStamps = getTimeStampsNormalized(masters, sqzV, tScale);

	// the base master is the bottom one that is not virtual
	double minT = maxDouble();
	for (PatchNode* m : masters){
		if (!m->hasTag(EDGE_VIRTUAL_TAG) && masterTimeStamps[m->mID] < minT){
			baseMaster = m;
			minT = masterTimeStamps[m->mID];
		}
	}

	// projected masters on the base master
	QMap<QString, Geom::Rectangle2> proj_rects;
	Geom::Rectangle base_rect = baseMaster->mPatch;
	for (PatchNode* m : masters)
	{
		// skip virtual 
		if (m->hasTag(EDGE_VIRTUAL_TAG)) continue;

		proj_rects[m->mID] = base_rect.get2DRectangle(m->mPatch);
	}

	// check each pair of masters
	for (int i = 0; i < masters.size(); i++)
	{
		// skip virtual 
		if (masters[i]->hasTag(EDGE_VIRTUAL_TAG)) continue;

		for (int j = i+1; j < masters.size(); j++)
		{
			// skip virtual 
			if (masters[j]->hasTag(EDGE_VIRTUAL_TAG)) continue;

			// two non-virtual masters
			QString mi = masters[i]->mID;
			QString mj = masters[j]->mID;

			// test intersection between projections
			if (Geom::IntrRect2Rect2::test(proj_rects[mi], proj_rects[mj]))
			{
				double ti = masterTimeStamps[mi];
				double tj = masterTimeStamps[mj];

				// <up, down>
				if (ti > tj) 
				{
					masterOrderGreater[mi] << mj;
					masterOrderLess[mj] << mi;
				}
				else 
				{
					masterOrderGreater[mj] << mi;
					masterOrderLess[mi] << mj;
				}	
			}
		}
	}
}

UnitScaff* DecScaff::getSelUnit()
{
	if (selUnitIdx >= 0 && selUnitIdx < units.size())
		return units[selUnitIdx];
	else
		return nullptr;
}

Scaffold* DecScaff::activeScaffold()
{
	UnitScaff* selUnit = getSelUnit();
	if (selUnit) return selUnit->activeScaffold();
	else		  return this;
}

QStringList DecScaff::getUnitLabels()
{
	QStringList labels;
	for (UnitScaff* l : units)
		labels.append(l->mID);

	// append string to select none
	labels << "--none--";

	return labels;
}

void DecScaff::selectUnit( QString id )
{
	// select layer named id
	selUnitIdx = -1;
	for (int i = 0; i < units.size(); i++){
		if (units[i]->mID == id){
			selUnitIdx = i;
			break;
		}
	}

	// disable selection on chains
	if (getSelUnit())
		getSelUnit()->selectChain("");
}

void DecScaff::storeDebugInfo(Scaffold* kf, int uidx)
{
	// assert
	if (!kf || uidx < 0 || uidx >= units.size()) return;

	// the active unit
	UnitScaff* unit = units[uidx];

	// highlight active parts
	for (QString nid : unit->getSlnSlaveParts())
	{
		Structure::Node* node = kf->getNode(nid);
		if (node) node->addTag(ACTIVE_NODE_TAG);
	}

	// aabb constraint
	kf->visDebug.addBox(ParSingleton::instance()->aabbCstr, Qt::cyan);

	// obstacles
	kf->visDebug.addPoints(unit->getCurrObstacles(), Qt::blue);

	// solution
	kf->visDebug.addRectangles(unit->getCurrSlnFRs(), Qt::green);
}

Scaffold* DecScaff::genKeyframe( double t )
{
	// folded units
	QVector<Scaffold*> uKeyframes;
	int activeUnitIdx = -1;
	for (int i = 0; i < units.size(); i++)
	{
		UnitScaff* unit = units[i];

		Scaffold* uk = nullptr;
		if (unit->mFoldDuration.start >= 1)
		{
			// the unit has not been foldabilized
			uk = (Scaffold*)unit->clone();
		}
		else
		{
			// foldabilized unit
			double lt = unit->mFoldDuration.getLocalTime(t);
			uk = unit->getKeyframe(lt, false);

			// active unit
			if (lt > 0 && lt < 1) activeUnitIdx = i;
		}

		uKeyframes << uk;
	}

	// keyframe via combination: aligned on base master
	Scaffold *keyframe = new Scaffold(uKeyframes, baseMaster->mID, masterUnitMap);

	// delete folded blocks
	for (Scaffold* unit : uKeyframes) delete unit;

	// debug
	storeDebugInfo(keyframe, activeUnitIdx);

	return keyframe;
}

void DecScaff::genKeyframes()
{
	keyframes.clear();

	int N = ParSingleton::instance()->nbKeyframes;
	double step = 1.0 / (N-1);
	for (int i = 0; i < N; i++)
	{
		Scaffold* kf = genKeyframe(i * step);
		if(!kf) return;

		// color
		for (ScaffNode* n : kf->getScaffNodes())
		{
			double grey = 240;
			QColor c = (n->hasTag(ACTIVE_NODE_TAG) && !n->hasTag(MASTER_TAG)) ? 
				QColor::fromRgb(255, 110, 80) : QColor::fromRgb(grey, grey, grey);
			n->setColor(c);
		}

		// unwrap bundles to view real parts
		kf->unwrapBundleNodes();
		kf->removeNodesWithTag(EDGE_VIRTUAL_TAG);

		// save the key frame
		keyframes << kf;
	}
}

SuperShapeKf* DecScaff::getSuperShapeKf( double t )
{
	// super key frame for each block
	QVector<Scaffold*> uSuperKf;
	for (int i = 0; i < units.size(); i++)
	{
		UnitScaff* unit = units[i];

		Scaffold* uk_super;
		if (unit->mFoldDuration.start >= 1)
		{// the unit has not been foldabilized
			uk_super = (Scaffold*)unit->clone();
		}
		else
		{
			double lt = unit->mFoldDuration.getLocalTime(t);
			uk_super = unit->getSuperKeyframe(lt);
		}

		uSuperKf << uk_super;
	}

	// combine using regular masters shared between units
	// whilst all super masters and slaves will be cloned into the superKf
	// based on the base master
	Scaffold *superKf = new Scaffold(uSuperKf, baseMaster->mID, masterUnitMap);

	// create super shape key frame
	// super masters sharing common regular masters will be grouped
	SuperShapeKf* superShapeKf = new SuperShapeKf(superKf, masterOrderGreater);

	// garbage collection
	delete superKf;
	for (int i = 0; i < uSuperKf.size(); i++) delete uSuperKf[i];

	// return
	return superShapeKf;
}

void DecScaff::foldabilize()
{
	// initialization
	for (UnitScaff* u : units)
	{
		u->initFoldSolution();
		u->mFoldDuration.set(1.0, 2.0); // t > 1.0 means not be folded
	}

	// choose best free unit
	std::cout << "\n============================="
			  << "\n============START============\n";
	double currTime = 0.0;
	SuperShapeKf* currKeyframe = getSuperShapeKf(currTime);
	UnitScaff* next_unit = getBestNextUnit(currTime, currKeyframe);

	{//debug
		//return;
		//int i = 0;
	}

	while (next_unit)
	{
		std::cout << "**\nBest next = " << next_unit->mID.toStdString() << "\n"
				  << "Foldabilizing unit: " << next_unit->mID.toStdString() << "\n";

		// foldabilize
		double timeLength = next_unit->getNbTopMasters() * timeScale;
		double nextTime = currTime + timeLength;
		next_unit->foldabilize(currKeyframe, TimeInterval(currTime, nextTime));

		{//debug
			//i++;
			//if (i == 3)
			//return;
		}


		std::cout << "\n-----------//-----------\n";

		// get best next
		currTime = nextTime;
		delete currKeyframe;
		currKeyframe = getSuperShapeKf(currTime);
		next_unit = getBestNextUnit(currTime, currKeyframe);
	}

	// interlocking units
	foldabilizeInterlockUnits(currTime, currKeyframe);

	// finish
	delete currKeyframe;
	std::cout << "\n============FINISH============\n";
}


void DecScaff::foldabilizeInterlockUnits(double currTime, SuperShapeKf* currKeyframe)
{
	// the map between old unit indices to the new ones
	QVector<int> newUnitIdx;
	QVector<UnitScaff*> oldUnits = units;
	units.clear();

	// delete interlocking units
	QVector<int> itlkUnitIdx;
	for (int i = 0; i < oldUnits.size(); i++){
		if (oldUnits[i]->mFoldDuration.start < 1){
			// units has been foldabilized
			units << oldUnits[i];
			newUnitIdx << units.size() - 1;
		}else{
			// interlocking units
			itlkUnitIdx << i;
			newUnitIdx << -1;
			delete oldUnits[i];
		}
	}

	// do nothing if no interlocking
	if (itlkUnitIdx.isEmpty()) return;
	std::cout << "\n=====MERGE INTERLOCKING======\n";

	// gather all interlocking slaves
	QSet<int> itlkSlaves;
	for (int ui : itlkUnitIdx){
		itlkSlaves += slaveClusters[ui];
		// map deleted unit to the new location
		newUnitIdx[ui] = units.size();
	}

	// merge them as single unit to foldabilize
	UnitScaff* mergedUnit = createUnit(itlkSlaves);
	units << mergedUnit;
	mergedUnit->initFoldSolution();
	mergedUnit->foldabilize(currKeyframe, TimeInterval(currTime, 1.0));


	// update master to unit map for keyframe generation
	for (QString mid : masterUnitMap.keys()){
		foreach(int ui_old, masterUnitMap[mid]){
			int ui_new = newUnitIdx[ui_old];
			masterUnitMap[mid].remove(ui_old);
			masterUnitMap[mid].insert(ui_new);
		}
	}
}


double DecScaff::foldabilizeUnit(UnitScaff* unit, double currTime, SuperShapeKf* currKf,
										double& nextTime, SuperShapeKf*& nextKf)
{
	// foldabilize nextUnit wrt the current super key frame
	double timeLength = unit->getNbTopMasters() * timeScale;
	nextTime = currTime + timeLength;
	double cost = unit->foldabilize(currKf, TimeInterval(currTime, nextTime));

	// get the next super key frame 
	nextKf = getSuperShapeKf(nextTime);

	// the normalized cost wrt. the importance
	cost *= unit->importance;
	return cost;
}

/**   currT                 nextT                 next2T
	    |------nextBlock------|------next2Block------|
   currKeyframe          nextKeyframe           next2Keyframe
**/  
// currKeyframe is a super keyframe providing context information
UnitScaff* DecScaff::getBestNextUnit(double currTime, SuperShapeKf* currKeyframe)
{
	// unfolded units
	QVector<UnitScaff*> unfoldedUnits;
	for (UnitScaff* u : units){
		if (!u->mFoldDuration.hasPassed(currTime))
			unfoldedUnits << u;
	}

	// trivial cases
	if (unfoldedUnits.isEmpty()) return nullptr;
	if (unfoldedUnits.size() == 1) return unfoldedUnits.front();

	// choose the best
	UnitScaff* nextBest = nullptr;
	double min_cost = maxDouble();
	for (UnitScaff* nextUnit : unfoldedUnits)
	{
		{//debug
			//if (nextUnit->mID != "TB_2") continue;
		}

		// foldabilize nextUnit
		std::cout << "*\nEvaluating unit: " << nextUnit->mID.toStdString() << "\n";
		double nextTime = -1.0;
		SuperShapeKf* nextKeyframe = nullptr;
		TimeInterval origNextTi = nextUnit->mFoldDuration; // back up
		double nextCost = foldabilizeUnit(nextUnit, currTime, currKeyframe, nextTime, nextKeyframe);

		{ // debug

			//nextUnit->genDebugInfo();
			//return nullptr;
		}
		
		// the folding of nextUnit must be valid, otherwise skip further evaluation
		if (nextKeyframe->isValid())
		{
			std::cout << "Look forward...\n";
			// cost of folding remaining unfolded units
			for (UnitScaff* next2Unit : unfoldedUnits)
			{
				// skip nextUnit
				if (next2Unit == nextUnit) continue;

				// foldabilize next2Unit
				std::cout << "==> " << next2Unit->mID.toStdString() << "\n";
				double next2Time;
				SuperShapeKf* next2Keyframe;
				TimeInterval origNext2Ti = next2Unit->mFoldDuration;
				double next2Cost = foldabilizeUnit(next2Unit, nextTime, nextKeyframe, next2Time, next2Keyframe);

				// invalid block folding generates large cost as being deleted
				if (!next2Keyframe->isValid()) next2Cost = next2Unit->importance; // deletion cost = 1.0

				delete next2Keyframe; // garbage collection
				next2Unit->mFoldDuration = origNext2Ti; // restore time interval
			}

			// debug info
			std::cout << "Cost = " << nextCost << std::endl;

			// nextBlock with lowest cost wins
			if (nextCost < min_cost - ZERO_TOLERANCE_LOW){
				min_cost = nextCost;
				nextBest = nextUnit;
			}
		}
		else
		{
			std::cout << "Invalid folding.\n";
		}

		{ // debug
			//return nullptr;
		}

		delete nextKeyframe; // garbage collection
		nextUnit->mFoldDuration = origNextTi;// restore time interval
	}

	// found the best next block in terms of introducing most free space for others
	return nextBest;
}


Scaffold* DecScaff::getSelKeyframe()
{
	if (keyframeIdx >= 0 && keyframeIdx < keyframes.size())
		return keyframes[keyframeIdx];
	else return nullptr;
}

void DecScaff::selectKeyframe( int idx )
{
	if (idx >= 0 && idx < keyframes.size())
		keyframeIdx = idx;
}

double DecScaff::getConnectThr()
{
	double connThrRatio = ParSingleton::instance()->connThrRatio;
	return connThrRatio * computeAABB().radius();
}

void DecScaff::computeUnitImportance()
{
	double totalA = 0;
	for (UnitScaff* u : units)
		totalA += u->getTotalSlaveArea();

	for (UnitScaff* u : units)
		u->setImportance(u->getTotalSlaveArea() / totalA);
}