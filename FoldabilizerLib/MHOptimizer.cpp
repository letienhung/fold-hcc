#include "MHOptimizer.h"
#include "IntrBoxBox.h"
#include <QTime>

using namespace Geom;

MHOptimizer::MHOptimizer(Graph* graph)
{
	this->hccGraph = graph;

	this->alwaysAccept = false;
	this->distWeight = 0.5;
	this->temperature = 100;
	this->targetVPerc = 0.5;
	this->resCollProb = 1.0;

	isReady = false;
}

void MHOptimizer::initialize()
{
	// set seed to random generator
	qsrand(QTime::currentTime().msec());

	origV = hccGraph->getAabbVolume();
	origMtlV = hccGraph->getMtlVolume();

	currState = hccGraph->getState();
	currCost = cost();

	this->jumpCount = 0;
	isReady = true;

	// new page
	qDebug() << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		<< "Initialized: currCost = " << currCost;
}

void MHOptimizer::jump()
{ 
	// clear debug points
	foreach(Node* n, hccGraph->nodes) n->debug_points.clear();

	// initialize
	if (!isReady) initialize();

	// propose jump and resolve collision
	proposeJump();
	resolveCollision();

	// accept new state by probability
	if (acceptJump()){
		currState = hccGraph->getState();
		currCost = cost();
		qDebug() << "\t\tACCEPTED. currCost = " << currCost;
	}else{
		hccGraph->setState(currState);
		hccGraph->restoreConfiguration();
		qDebug() << "\t\tREJECTED. currCost = " << currCost;
	}

	jumpCount++;
	emit(hccChanged());
}

double MHOptimizer::cost()
{
	// gas between current state and target
	double aabbVolume = hccGraph->getAabbVolume();
	double gap = aabbVolume / origV - targetVPerc;
	double cost0 = RANGED(0, gap, 1);

	// distortion on material lost
	double mtlV = hccGraph->getMtlVolume();
	double distortion = 1 - mtlV / origMtlV;
	double cost1 = pow(distortion, distWeight);

	return cost0 + cost1;
}

void MHOptimizer::proposeJump()
{
	// hot analysis
	hccGraph->hotAnalyze();
	QVector<Link*> hotLinks = hccGraph->getHotLinks(true);
	int hotID = uniformDiscreteDistribution(0, hotLinks.size());
	Link* plink = hotLinks[hotID];

	// hinge id
	int hingeID = uniformDiscreteDistribution(0, plink->nbHinges());
	plink->setActiveHingeId(hingeID);
	this->propHinge = plink->activeHinge();

	// switch hinge between two states: open or folded
	double old_angle = propHinge->angle;
	switch(propHinge->state)
	{
	case Hinge::FOLDED:
		propHinge->setState(Hinge::UNFOLDED);
		break;
	case Hinge::UNFOLDED:
		propHinge->setState(Hinge::FOLDED);
		break;
	case Hinge::HALF_FOLDED:
		int new_state = winByChance(0.5) ? 
			Hinge::FOLDED : Hinge::UNFOLDED;
		propHinge->setState(new_state);
		break;
	}

	qDebug() << "Jump" << jumpCount << "(" << plink->id << "): " 
		<< radians2degrees(old_angle) << " => " << radians2degrees(propHinge->angle);

	// restore configuration according to new parameters
	hccGraph->restoreConfiguration();
	emit(hccChanged());
}

bool MHOptimizer::acceptJump()
{
	if (alwaysAccept) return true;

	if (hccGraph->detectCollision())
		return false;

	double propCost = cost();
	if (propCost < currCost)
		return true;
	else
	{
		double a = pow(currCost/propCost, temperature);
		return winByChance(a);
	}
}

void MHOptimizer::run()
{

}

void MHOptimizer::resolveCollision()
{
	// no collision
	if (!hccGraph->detectCollision()) return;

	if (winByChance(this->resCollProb))
	{
		if (!propHinge->node1->collisionList.isEmpty())
			this->resolveCollision(propHinge->node1);

		if (!propHinge->node2->collisionList.isEmpty())
			this->resolveCollision(propHinge->node2);
	}

	hccGraph->restoreConfiguration();
}

void MHOptimizer::resolveCollision( Node* pn )
{
	// get intr points between pn and others
	QVector<Vector3> intrPnts;
	foreach(Node* cn, pn->collisionList)
		intrPnts += IntrBoxBox::sampleIntr(cn->mBox, pn->mBox);
	pn->debug_points = intrPnts;

	// analyze the width of frontier
	Vector4 fw = pn->mBox.calcFrontierWidth(propHinge->hX, propHinge->hZ, intrPnts);
	qDebug() <<"\tFrontier width: " << qstr(fw);

	// near-end
	double nfw = fw[1];
	if (nfw > ZERO_TOLERANCE_LOW)
	{
		return;
		qDebug() << "Potential split.";
	}

	// far-end
	double ffw = fw[0];
	if (ffw > 0 && ffw < 0.25)
	{
		int fe_aid = pn->mBox.getAxisID(propHinge->hX);
		pn->scale(fe_aid, 1 - ffw/2);
		qDebug() << "Shrunk far end.";
	}

	// two sides
	double lfw = fw[2], rfw = fw[3];
	double sfw = Max(lfw, rfw);
	if (sfw > 0 && sfw < 0.25)
	{
		int s_aid = pn->mBox.getAxisID(propHinge->hZ);
		pn->scale(s_aid, 1 - sfw);
		qDebug() << "Shrunk two sides.";
	}
}

void MHOptimizer::debug()
{
	int nid = uniformDiscreteDistribution(0, hccGraph->nbNodes());
	hccGraph->getNode(nid)->scale(uniformRealDistribution());
}
