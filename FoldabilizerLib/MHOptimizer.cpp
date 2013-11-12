#include "MHOptimizer.h"
#include "IntersectBoxBox.h"

MHOptimizer::MHOptimizer(Graph* graph)
{
	this->hccGraph = graph;

	isReady = false;
}

void MHOptimizer::initialize()
{
	qsrand(QTime::currentTime().msec());

	originalAabbVolume = hccGraph->getAabbVolume();
	originalMaterialVolume = hccGraph->getMaterialVolume();

	typeProbability << 1.0 << 0.0;

	currState = hccGraph->getState();
	currCost = cost();

	isReady = true;

	qDebug() << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
}

void MHOptimizer::jump()
{ 
	if (hccGraph->isEmpty()) return;
	if (!isReady) initialize();
	
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
}

double MHOptimizer::cost()
{
	// gain on foldability
	double aabbVolume = hccGraph->getAabbVolume();
	double gain = 1 - aabbVolume / originalAabbVolume;

	// distortion on material lost
	double materialVolume = hccGraph->getMaterialVolume();
	double distortion = 1 - materialVolume / originalMaterialVolume;

	// cost should be non-negative
	double alpha = 0.2;
	double c = alpha * distortion - (1 - alpha) * gain;
	double nc = c + 0.8;
	return nc;
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

			qDebug() << "[Proposal move] Angle of " << plink->id.toStdString().c_str() << ": " << radians2degrees(old_angle) << " => " << radians2degrees(new_angle);
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

			qDebug() << "[Proposal move] Scale factor[" << axisID << "] of " << pnode->mID.toStdString().c_str() << ": " << old_factor << " => " << new_factor;
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

	double c = cost();
	if (c < currCost)
		return true;
	else
	{
		double a = pow(currCost/c, 6);
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


