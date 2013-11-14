#include "MHOptimizer.h"
#include "IntersectBoxBox.h"

MHOptimizer::MHOptimizer(Graph* graph)
{
	this->hccGraph = graph;

	this->targetVolumePercentage = 0.5;
	this->costWeight = 0.5;
	this->temperature = 100;
	this->setLinkProbability(0.8);

	isReady = false;
}

void MHOptimizer::initialize()
{
	// set seed to random generator
	qsrand(QTime::currentTime().msec());

	originalAabbVolume = hccGraph->getAabbVolume();
	originalMaterialVolume = hccGraph->getMaterialVolume();

	currState = hccGraph->getState();
	currCost = cost();

	this->jumpCount = 0;
	isReady = true;

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
	
}

double MHOptimizer::cost()
{
	// cost foldability
	double aabbVolume = hccGraph->getAabbVolume();
	double cost1 = aabbVolume / originalAabbVolume - targetVolumePercentage;
	if (cost1 < 0) cost1 = 0;

	// distortion on material lost
	double materialVolume = hccGraph->getMaterialVolume();
	double distortion = 1 - materialVolume / originalMaterialVolume;

	return cost1 + pow(distortion, costWeight);
}

void MHOptimizer::proposeJump()
{
	int jump_type = discreteDistribution(typeProbability);
	switch (jump_type)
	{
	case 0: 
		{
			// hinge id
			int linkID = uniformDiscreteDistribution(0, hccGraph->nbLinks());
			Link* plink = hccGraph->links[linkID];
			if (plink->isNailed || plink->isBroken) return;

			// angle
			double stddev = plink->angle_suf / 6; // 3-\delta coverage of 99.7%
			double old_angle = plink->angle;
			double new_angle = periodicalRanged(0, plink->angle_suf, normalDistribution.generate(old_angle, stddev));
			plink->angle = new_angle;

			qDebug() << "[Jump" << jumpCount << "] Angle of " << plink->id.toStdString().c_str() << ": " 
				<< radians2degrees(old_angle) << " => " << radians2degrees(new_angle);
		}
		break;
	case 1: 
		{
			// cuboid id
			int nodeID = uniformDiscreteDistribution(0, hccGraph->nbNodes());
			Node* pnode = hccGraph->nodes[nodeID];

			// axis id
			int axisID = uniformDiscreteDistribution(0, 3);

			// scale factor
			double stddev = 1 / 12.0;
			double old_factor = pnode->scaleFactor[axisID];
			double new_factor = periodicalRanged(0.5, 1.0, normalDistribution.generate(old_factor, stddev));
			pnode->scaleFactor[axisID] = new_factor;

			qDebug() << "[Jump" << jumpCount << "] Scale factor[" << axisID << "] of " << pnode->mID.toStdString().c_str() << ": " 
				<< old_factor << " => " << new_factor;
		}
		break;
	default:
		break;
	}

	// restore configuration according to new parameters
	hccGraph->restoreConfiguration();
}

bool MHOptimizer::acceptJump()
{
	if (!isCollisionFree())
		return false;

	double propCost = cost();
	if (propCost < currCost)
		return true;
	else
	{
		double a = pow(currCost/propCost, temperature);
		double r = uniformRealDistribution();
		return r < a;
	}

	//return (cost() < currCost);
	//return true;
}

bool MHOptimizer::isCollisionFree()
{
	// clear highlights
	foreach(Node* n, hccGraph->nodes) 
		n->isHighlight = false;

	// detect collision between each pair of cuboids
	bool isFree = true;
	for (int i = 0; i < hccGraph->nbNodes()-1; i++){
		for (int j = i+1; j < hccGraph->nbNodes(); j++)
		{
			Node* n1 = hccGraph->nodes[i];
			Node* n2 = hccGraph->nodes[j];

			if (IntersectBoxBox::test(n1->getRelaxedBox(), n2->getRelaxedBox()))
			{
				n1->isHighlight = true;
				n2->isHighlight = true;
				isFree = false;
			}
		}
	}

	return isFree;
}

void MHOptimizer::setLinkProbability( double lp )
{
	typeProbability.resize(0);
	typeProbability << lp << 1 - lp;
}


