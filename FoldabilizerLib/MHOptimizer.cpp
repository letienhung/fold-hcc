#include "MHOptimizer.h"

#include "Numeric.h"
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

	typeProbability << 0.8 << 0.2;

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

			qDebug() << "[Proposal move] Angle of " << plink->id.toStdString().c_str() << ": " << radians2degrees(old_angle) << " => " << radians2degrees(new_angle);
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
	isCollisionFree();

	//return (cost() < currCost);
	return true;
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


