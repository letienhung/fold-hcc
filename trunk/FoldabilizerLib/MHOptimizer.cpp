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
	qDebug() << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
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

	if (!hccGraph->detectCollision())
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



	// resolve collision of nodes incident to propHinge
	Node* pn1 = propHinge->node1;
	QVector<Vector3> intrPnts;
	foreach(Node* cn, pn1->collisionList)
		intrPnts += IntrBoxBox::sampleIntr(cn->mBox, pn1->mBox);
	pn1->debug_points = intrPnts;

	// analyze the position of intrPnts and 

	Node* pn2 = propHinge->node2;
}
