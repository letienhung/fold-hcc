#include "TChain.h"
#include "Hinge.h"
#include "Numeric.h"

TChain::TChain( PatchNode* master, FdNode* slave)
	:ChainGraph(slave, master, NULL)// ChainGraph has cloned slave and master
{
	// type
	mType = T_CHAIN;
}

Geom::SectorCylinder TChain::getFoldingVolume( FoldOption* fn )
{
	// hinge axis and rightV
	int hidx = fn->hingeIdx;
	Geom::Segment axisSeg = rootJointSegs[hidx];
	Vector3 rightV = rootRightVs[hidx];
	if (!fn->rightSide) rightV *= -1;

	return Geom::SectorCylinder(axisSeg, chainUpSeg, rightV);
}

void TChain::resolveCollision( FoldOption* fn )
{
	Geom::SectorCylinder sfVolume = fn->properties["sfVolume"].value<Geom::SectorCylinder>();

	// impossible splits
	// to do: resolve collision cooperatively
	if (sfVolume.Radius <= 0) return;  

	int nbPart = ceil( getLength() / (sfVolume.Radius + ZERO_TOLERANCE_LOW) );
	if (nbPart != mParts.size()) createChain(nbPart);
}

QVector<FoldOption*> TChain::generateFoldOptions()
{
	QVector<FoldOption*> options;

	// #splits: 0 --> 2
	for (int n = 0; n <= 2; n++)
	{
		// patch chain
		if (mOrigSlave->mType == FdNode::PATCH)
		{
			// shrink: scale level : 1 --> 5 : 20% --> 100%
			int nbScales = 5;
			for (int i = 1; i <= nbScales; i++)
			{
				double step = 1.0/double(nbScales);
				double scale = step * i;

				// position
				for (int j = 0; j <= nbScales - i; j++)
				{
					double position = step * j;

					// left
					QString fnid1 = this->mID + "_" + QString::number(options.size());
					FoldOption* fn1 = new FoldOption(0, false, scale, position, n, fnid1);
					options.push_back(fn1);

					// right
					QString fnid2 = this->mID + "_" + QString::number(options.size());
					FoldOption* fn2 = new FoldOption(0, true, scale, position, n, fnid2);
					options.push_back(fn2);
				}
			}
		}
		// rod chain
		else
		{
			// root segment id
			for (int j = 0; j < 2; j++)
			{
				// left
				QString fnid1 = this->mID + "_" + QString::number(options.size());
				FoldOption* fn1 = new FoldOption(j, false, 1.0, 0.0, n, fnid1);
				options.push_back(fn1);

				// right
				QString fnid2 = this->mID + "_" + QString::number(options.size());
				FoldOption* fn2 = new FoldOption(j, true, 1.0, 0.0, n, fnid2);
				options.push_back(fn2);
			}

		}
	}

	// fold volume
	foreach (FoldOption* fn, options)
	{
		Geom::SectorCylinder fVolume = this->getFoldingVolume(fn);
		fn->properties["fVolume"].setValue(fVolume);
	}

	return options;
}

void TChain::modify( FoldOption* fn )
{

}
