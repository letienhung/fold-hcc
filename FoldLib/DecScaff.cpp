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

DecScaff::DecScaff(QString id, Scaffold* scaffold, Vector3 v, double connThr)
	: Scaffold(*scaffold), baseMaster(nullptr) // clone the FdGraph
{
	path = QFileInfo(path).absolutePath();
	mID = id;
	sqzV = v;

	// threshold
	connThrRatio = connThr;

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
	selBlockIdx = -1;
	keyframeIdx = -1;
	//////////////////////////////////////////////////////////////////////////
	//QVector<Geom::Segment> normals;
	//for (PatchNode* m, masters)
	//	normals << Geom::Segment(m->mPatch.Center, m->mPatch.Center + 10 * m->mPatch.Normal);
	//addDebugSegments(normals);
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
		&& (ms[0]->hasTag(EDGE_ROD_TAG)
		|| ms[1]->hasTag(EDGE_ROD_TAG)))// one master is edge rod
	{
		unit = new TUnitScaff("TB_" + id, ms, ss, mPairs);
	}
	else if (ms.size() == 2				// two masters
		&& !ms[0]->hasTag(EDGE_ROD_TAG)
		&& !ms[1]->hasTag(EDGE_ROD_TAG) // both masters are real
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
	QMap<QString, double> masterTimeStamps = getTimeStampsNormalized(masters, sqzV, tScale);

	// the base master is the bottom one that is not virtual
	double minT = maxDouble();
	for (PatchNode* m : masters){
		if (!m->hasTag(EDGE_ROD_TAG) && masterTimeStamps[m->mID] < minT){
			baseMaster = m;
			minT = masterTimeStamps[m->mID];
		}
	}

	// projected masters on the base master
	QMap<QString, Geom::Rectangle2> proj_rects;
	Geom::Rectangle base_rect = baseMaster->mPatch;
	for (PatchNode* m : masters)
		proj_rects[m->mID] = base_rect.get2DRectangle(m->mPatch);

	// check each pair of masters
	for (int i = 0; i < masters.size(); i++){
		for (int j = i+1; j < masters.size(); j++)
		{
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
	if (selBlockIdx >= 0 && selBlockIdx < units.size())
		return units[selBlockIdx];
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
	selBlockIdx = -1;
	for (int i = 0; i < units.size(); i++){
		if (units[i]->mID == id){
			selBlockIdx = i;
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
	for (Structure::Node* n : unit->nodes)
	{
		Structure::Node* an = kf->getNode(n->mID);
		if (an)	an->addTag(ACTIVE_NODE_TAG);
	}

	// the offset of active unit in the scaffold
	PatchNode* newUnitBase = (PatchNode*)kf->getNode(unit->baseMaster->mID);
	Vector3 oldPos = unit->baseMaster->center();
	Vector3 newPos = newUnitBase->center();
	Vector3 offset = newPos - oldPos;

	// aabb constraint
	kf->debugBoxes << unit->aabbCstr;

	// aabb constraint projection
	//kf->appendToVectorProperty(DEBUG_SEGS, newUnitBase->mPatch.get3DRectangle(unit->aabbCstrProj).getEdgeSegments());

	// obstacles
	QVector<Vector3> obs = unit->getObstacles();
	for (Vector3 p : obs) p += offset;
	kf->debugPntsB << obs;

	// fold options
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
			uk = unit->getKeyframe(lt, true);

			// active unit
			if (lt > 0 && lt < 1) activeUnitIdx = i;
		}

		uKeyframes << uk;
	}

	// keyframe via combination
	Scaffold *keyframe = new Scaffold(uKeyframes, baseMaster->mID, masterUnitMap);

	// delete folded blocks
	for (Scaffold* unit : uKeyframes) delete unit;

	// debug
	storeDebugInfo(keyframe, activeUnitIdx);

	return keyframe;
}

void DecScaff::genKeyframes( int N )
{
	keyframes.clear();

	double step = 1.0 / (N-1);
	for (int i = 0; i < N; i++)
	{
		Scaffold* kf = genKeyframe(i * step);
		if(!kf) return;

		keyframes << kf;

		kf->unwrapBundleNodes();
		kf->hideEdgeRods();

		// color
		for (ScaffNode* n : kf->getScaffNodes())
		{
			double grey = 240;
			QColor c = (n->hasTag(ACTIVE_NODE_TAG) && !n->hasTag(MASTER_TAG)) ? 
				QColor::fromRgb(255, 110, 80) : QColor::fromRgb(grey, grey, grey);
			c.setAlphaF(0.78);
			n->mColor = c;
		}
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

	// combine
	Scaffold *keyframe = new Scaffold(uSuperKf, baseMaster->mID, masterUnitMap);

	// create shape super key frame
	SuperShapeKf* ssKeyframe = new SuperShapeKf(keyframe, masterOrderGreater);

	// garbage collection
	delete keyframe;
	for (int i = 0; i < uSuperKf.size(); i++) delete uSuperKf[i];

	// return
	return ssKeyframe;
}

void DecScaff::foldabilize()
{
	// initialization
	for (UnitScaff* u : units)
		u->mFoldDuration.set(1.0, 2.0); // t > 1.0 means not be folded

	// choose best free block
	std::cout << "\n============================="
			  << "\n============START============\n";
	double currTime = 0.0;
	SuperShapeKf* currKeyframe = getSuperShapeKf(currTime);
	UnitScaff* next_unit = getBestNextUnit(currTime, currKeyframe);


	//return;


	while (next_unit)
	{
		std::cout << "**\nBest next = " << next_unit->mID.toStdString() << "\n"
				  << "Foldabilizing unit: " << next_unit->mID.toStdString() << "\n";

		// foldabilize
		double timeLength = next_unit->getNbTopMasters() * timeScale;
		double nextTime = currTime + timeLength;
		next_unit->foldabilize(currKeyframe, TimeInterval(currTime, nextTime));

		std::cout << "\n-----------//-----------\n";

		// get best next
		currTime = nextTime;
		delete currKeyframe;
		currKeyframe = getSuperShapeKf(currTime);
		next_unit = getBestNextUnit(currTime, currKeyframe);
	}

	// remaining blocks (if any) are interlocking
	QVector<UnitScaff*> units_copy = units;
	units.clear();
	QVector<int> intlkUnitIdx;
	for (int uid = 0; uid < units_copy.size(); uid ++)
	{
		UnitScaff* u = units_copy[uid];
		if (u->mFoldDuration.start < 1) units << u;
		else intlkUnitIdx << uid;
	}

	// merge them as single block to foldabilize
	if (!intlkUnitIdx.isEmpty())
	{
		std::cout << "\n=====MERGE INTERLOCKING======\n";
		QSet<int> sCluster;
		for (int uidx : intlkUnitIdx)
			sCluster += slaveClusters[uidx];
		
		UnitScaff* mergedBlock = createUnit(sCluster);
		mergedBlock->foldabilize(currKeyframe, TimeInterval(currTime, 1.0));
	}

	delete currKeyframe;
	std::cout << "\n============FINISH============\n";
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
		//if (nextUnit->mID != "ZB_1") continue;

		// foldabilize nextUnit
		std::cout << "*\nEvaluating unit: " << nextUnit->mID.toStdString() << "\n";
		double nextTime = -1.0;
		SuperShapeKf* nextKeyframe = nullptr;
		TimeInterval origNextTi = nextUnit->mFoldDuration; // back up
		double nextCost = foldabilizeUnit(nextUnit, currTime, currKeyframe, nextTime, nextKeyframe);

		//nextUnit->genDebugInfo();
		//return nullptr;
		
		// the folding of nextUnit must be valid, otherwise skip further evaluation
		if (nextKeyframe->isValid(sqzV))
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
				if (!next2Keyframe->isValid(sqzV)) next2Cost = next2Unit->importance; // deletion cost = 1.0

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