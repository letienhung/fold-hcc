#include "MHOptimizer.h"

#include "Numeric.h"


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

	typeProbability << 0.6 << 0.4;

	currState = hccGraph->getState();
	currCost = cost();

	isReady = true;
}

void MHOptimizer::jump()
{ 
	if (hccGraph->isEmpty()) return;
	if (!isReady) initialize();

	proposeJump();
	if (doesAcceptJump())
	{
		currState = hccGraph->getState();
		currCost = cost();

		qDebug() << "Accepted the proposed jump. currCost = " << currCost;
	}
	else
	{
		hccGraph->setState(currState);
		hccGraph->restoreConfiguration();
		qDebug() << "Rejected the proposed jump. currCost = " << currCost;
	}
}

double MHOptimizer::cost()
{
	double aabbVolume = hccGraph->getAabbVolume();
	double gain = 1 - aabbVolume / originalAabbVolume;

	double materialVolume = hccGraph->getMaterialVolume();
	double distortion = 1 - materialVolume / originalMaterialVolume;

	return distortion - gain;
}

void MHOptimizer::proposeJump()
{
	int jump_type = randDiscrete(typeProbability);
	switch (jump_type)
	{
	case 0: // hinge angle
		{
			int linkID = randDiscreteUniform(hccGraph->nbLinks());
			Link* plink = hccGraph->links[linkID];
			if (plink->isNailed || plink->isBroken) return;
			double old_angle = plink->angle;
			double new_angle = randUniform(0, plink->angle_suf);
			plink->angle = new_angle;

			qDebug() << "Angle of " << plink->id << ": " << radians2degrees(old_angle) << " => " << radians2degrees(new_angle);
		}
		break;
	case 1: // cuboid scale
		{
			int nodeID = randDiscreteUniform(hccGraph->nbNodes());
			Node* pnode = hccGraph->nodes[nodeID];

			int axisID = randDiscreteUniform(3);
			double old_factor = pnode->scaleFactor[axisID];
			double new_factor = randUniform(0.5, 1);
			pnode->scaleFactor[axisID] = new_factor;

			qDebug() << "Scale factor[" << axisID << "] of " << pnode->mID << ": " << old_factor << " => " << new_factor;
		}
		break;
	default:
		break;
	}

	// restore configuration according to new parameters
	hccGraph->restoreConfiguration();
}

bool MHOptimizer::doesAcceptJump()
{
	return (cost() < currCost);
}


