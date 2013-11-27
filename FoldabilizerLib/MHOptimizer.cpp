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
	if (!isReady) initialize();
	if (hccGraph->isEmpty()) return;


	proposeJump();
	if (acceptJump())
	{
		currState = hccGraph->getState();
		currCost = cost();

		qDebug() << "\t\tACCEPTED. currCost = " << currCost;
	}
	else
	{
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
	Hinge* phinge = plink->activeHinge();

	// switch hinge between two states: open or folded
	double old_angle = phinge->angle;
	switch(phinge->state)
	{
	case Hinge::FOLDED:
		phinge->setState(Hinge::UNFOLDED);
		break;
	case Hinge::UNFOLDED:
		phinge->setState(Hinge::FOLDED);
		break;
	case Hinge::HALF_FOLDED:
		int new_state = winByChance(0.5) ? 
			Hinge::FOLDED : Hinge::UNFOLDED;
		phinge->setState(new_state);
		break;
	}

	qDebug() << "Jump" << jumpCount << "(" << plink->id << "): " 
		<< radians2degrees(old_angle) << " => " << radians2degrees(phinge->angle);

	// restore configuration according to new parameters
	hccGraph->restoreConfiguration();
	emit(hccChanged());
}

bool MHOptimizer::acceptJump()
{
	if (alwaysAccept) return true;

	if (!isCollisionFree())
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

bool MHOptimizer::isCollisionFree()
{
	// clear highlights
	foreach(Node* n, hccGraph->nodes) 
		n->isHighlight = false;

	// get boxes (shrunk)
	QVector<Box> nodeBoxes;
	foreach(Node* n, hccGraph->nodes)	{
		nodeBoxes.push_back(n->mBox);
		nodeBoxes.last().uniformScale(0.99);
	}

	// detect collision between each pair of cuboids
	bool isFree = true;
	int nbNodes = hccGraph->nbNodes();
	for (int i = 0; i < nbNodes-1; i++){
		for (int j = i+1; j < nbNodes; j++)
		{
			if (IntrBoxBox::test(nodeBoxes[i], nodeBoxes[j]))
			{
				hccGraph->getNode(i)->isHighlight = true;
				hccGraph->getNode(j)->isHighlight = true;
				isFree = false;
			}
		}
	}

	return isFree;
}

void MHOptimizer::run()
{

}
