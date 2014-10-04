#include "ZUnitScaff.h"
#include "TChainScaff.h"

ZUnitScaff::ZUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
	QVector< QVector<QString> >& mPairs) : UnitScaff(id, ms, ss, mPairs)
{
	// create the back up HUnit
	hUnit = new HUnitScaff("H_"+id, ms, ss, mPairs);

	// decompose
	sortMasters();
	createChains(ss, mPairs);
	computeChainImportances();

	// possible fold regions
	computeFoldRegions();
}

void ZUnitScaff::sortMasters()
{
	// the base and top master
	QMap<QString, double> masterTimeStamps =
		getTimeStampsNormalized(masters, masters.front()->mPatch.Normal, timeScale);
	if (masterTimeStamps[masters.front()->mID] < ZERO_TOLERANCE_LOW)
	{
		baseMaster = masters.front();	
		topMaster = masters.last();
	}
	else
	{
		baseMaster = masters.last();	
		topMaster = masters.front();
	}
}

void ZUnitScaff::createChains(QVector<ScaffNode*>& ss, QVector< QVector<QString> >&)
{
	for (ScaffNode* slave : ss)
	{
		ChainScaff* tc = new TChainScaff(slave, baseMaster, topMaster);
		tc->setFoldDuration(0, 1);
		chains << tc;
	}
}


void ZUnitScaff::computeFoldRegions()
{
	// the right direction is the rightSegV of first chain
	rightV = chains.front()->rightSegV;

	// 
}


double ZUnitScaff::foldabilizeWrt(ShapeSuperKeyframe* ssKeyframe)
{
	return 0;
}

Scaffold* ZUnitScaff::getKeyframe(double t, bool useThk)
{
	return nullptr;
}

void ZUnitScaff::applySolution()
{

}
