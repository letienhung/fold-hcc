#include "MHOptimizer.h"
#include "IntrBoxBox.h"
#include <QTime>

using namespace Geom;

MHOptimizer::MHOptimizer(Graph* graph)
{
	this->hccGraph = graph;

	// propose
	this->nbSigma = 6;
	this->setTypeProb(0.8, 0.2);
	this->switchHingeProb = 0.8;
	this->useHotProb = 1.0;

	// accept
	this->distWeight = 0.5;
	this->temperature = 100;
	this->alwaysAccept = false;

	// target
	this->targetV = 0.5;

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
	double cost1 = aabbVolume / originalAabbVolume - targetV;
	if (cost1 < 0) cost1 = 0;

	// distortion on material lost
	double materialVolume = hccGraph->getMaterialVolume();
	double distortion = 1 - materialVolume / originalMaterialVolume;

	return cost1 + pow(distortion, distWeight);
}

void MHOptimizer::proposeChangeHingeAngle()
{
	Link* plink = NULL;

	// get hot and code nodes
	hccGraph->hotAnalyze();
	QVector<Link*> hotLinks = hccGraph->getHotLinks(true);
	QVector<Link*> coldLinks = hccGraph->getHotLinks(false);

	// use hot nodes
	if (winByChance(useHotProb))
	{
		int hotID = uniformDiscreteDistribution(0, hotLinks.size());
		plink = hotLinks[hotID];
	}
	// use cold nodes
	else
	{
		if (coldLinks.isEmpty()) return;
		int coldID = uniformDiscreteDistribution(0, coldLinks.size());
		plink = coldLinks[coldID];
	}
	if (plink->isNailed || plink->isBroken) return;

	// hinge id
	if (winByChance(switchHingeProb))
	{
		int hingeID = uniformDiscreteDistribution(0, plink->nbHinges());
		plink->setActiveHingeId(hingeID);
	}
	Hinge* phinge = plink->activeHinge();

	// jump
	double stddev = phinge->maxAngle / 6; // 3-\delta coverage of 99.7%
	double old_angle = phinge->angle;
	double prop_angle = normalDistr.generate(old_angle, stddev);
	double new_angle = RANGED(0, prop_angle, phinge->maxAngle);
	phinge->angle = new_angle;

	qDebug() << "[Jump" << jumpCount << "] Angle of " << plink->id << ": " 
		<< radians2degrees(old_angle) << " => " << radians2degrees(new_angle);
}

void MHOptimizer::proposeDeformCuboid()
{
	Node* pnode = NULL;

	// get hot and code nodes
	hccGraph->hotAnalyze();
	QVector<Node*> hotNodes = hccGraph->getHotNodes(true);
	QVector<Node*> coldNodes = hccGraph->getHotNodes(false);

	// use hot nodes
	if (winByChance(useHotProb))
	{
		int hotID = uniformDiscreteDistribution(0, hotNodes.size());
		pnode = hotNodes[hotID];
	}
	// use cold nodes
	else
	{
		if (coldNodes.isEmpty()) return;
		int coldID = uniformDiscreteDistribution(0, coldNodes.size());
		pnode = coldNodes[coldID];
	}

	// axis id
	int axisID = uniformDiscreteDistribution(0, 3);

	// scale factor
	double stddev = 1 / 12.0;
	double old_factor = pnode->scaleFactor[axisID];
	double new_factor = periodicalRanged(0.5, 1.0, normalDistr.generate(old_factor, stddev));
	pnode->scaleFactor[axisID] = new_factor;

	qDebug() << "[Jump" << jumpCount << "] Scale factor[" << axisID << "] of " << pnode->mID.toStdString().c_str() << ": " 
		<< old_factor << " => " << new_factor;
}

void MHOptimizer::proposeJump()
{
	int jump_type = discreteDistribution(typeProb);
	if (jump_type == 0) 
		proposeChangeHingeAngle();
	else if(jump_type == 1)
		proposeDeformCuboid();

	// restore configuration according to new parameters
	hccGraph->restoreConfiguration();
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
		double r = uniformRealDistribution();
		return r < a;
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

void MHOptimizer::setTypeProb( QVector<double> &tp )
{
	typeProb = tp;
	double sum = 0;
	foreach(double p, typeProb) sum += p;
	for (int i = 0; i < typeProb.size(); i++)
		typeProb[i] /= sum;
}

void MHOptimizer::setTypeProb( double t0, double t1 )
{
	QVector<double> tp;
	tp << t0 << t1;

	this->setTypeProb(tp);
}