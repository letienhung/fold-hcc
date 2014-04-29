#include "HChain.h"
#include "Numeric.h"

HChain::HChain( FdNode* part, PatchNode* panel1, PatchNode* panel2)
	:ChainGraph(part, panel1, panel2)
{
	// type
	properties["type"] = "sandwich";

	createChain(2);
}

Geom::Rectangle2 HChain::getFoldRegion(FoldingNode* fn)
{
	// axis and rightV
	int hidx = fn->hingeIdx;
	Geom::Segment axisSeg1 = rootJointSegs[hidx];
	Vector3 rightV = rootRightVs[hidx];
	if (!fn->rightSide) rightV *= -1;

	// shrink along axisSeg
	double t0 = fn->position;
	double t1 = t0 + fn->scale;
	axisSeg1.crop(t0*2-1, t1*2-1); // [0,1] => [-1, 1]
	Geom::Segment axisSeg2 = axisSeg1;

	// shift the axis along rightV
	if (fn->nbsplit == 1)
	{
		double width = getLength() * 0.5;
		axisSeg2.translate(width * rightV);
	}
	else if (fn->nbsplit == 2)
	{
		double width = getLength() * 0.25;
		axisSeg1.translate(-width * rightV);
		axisSeg2.translate( width * rightV);
	}

	// conners of projected rectangle
	QVector<Vector2> conners;
	Geom::Rectangle& panel_rect = mMasters[0]->mPatch;
	conners << panel_rect.getProjCoordinates(axisSeg1.P0) 
		<< panel_rect.getProjCoordinates(axisSeg1.P1) 
		<< panel_rect.getProjCoordinates(axisSeg2.P1) 
		<< panel_rect.getProjCoordinates(axisSeg2.P0);

	return Geom::Rectangle2(conners);
}

Geom::Segment2 HChain::getFoldingAxis2D( FoldingNode* fn )
{
	Geom::Segment axisSeg = getJointSegment(fn);
	Geom::Rectangle& panel_rect = mMasters[0]->mPatch;

	return panel_rect.get2DSegment(axisSeg);
}

Geom::Segment HChain::getJointSegment( FoldingNode* fn )
{
	int hidx = fn->hingeIdx;
	int jidx = hidx / 2;
	return rootJointSegs[jidx];
}

QVector<FoldingNode*> HChain::generateFoldOptions()
{
	QVector<FoldingNode*> options;

	// patch chain
	if (mOrigPart->mType == FdNode::PATCH)
	{
		// #splits: 1 and 2
		for (int n = 1; n <= 2; n++)
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
					FoldingNode* fn1 = new FoldingNode(0, false, scale, position, n, fnid1);
					options.push_back(fn1);

					// right
					QString fnid2 = this->mID + "_" + QString::number(options.size());
					FoldingNode* fn2 = new FoldingNode(0, true, scale, position, n, fnid2);
					options.push_back(fn2);
				}
			}
		}
	}
	// rod chain
	else
	{
		// #splits: 1 and 2
		for (int n = 1; n <= 2; n++)
		{
			// root segment id
			for (int j = 0; j < 2; j++)
			{
				// left
				QString fnid1 = this->mID + "_" + QString::number(options.size());
				FoldingNode* fn1 = new FoldingNode(j, false, 1.0, 0.0, n, fnid1);
				options.push_back(fn1);

				// right
				QString fnid2 = this->mID + "_" + QString::number(options.size());
				FoldingNode* fn2 = new FoldingNode(j, true, 1.0, 0.0, n, fnid2);
				options.push_back(fn2);
			}
		}
	}

	// fold area
	foreach (FoldingNode* fn, options)
	{
		Geom::Rectangle2 fArea = this->getFoldRegion(fn);
		fn->properties["fArea"].setValue(fArea);
	}

	return options;
}

void HChain::modify( FoldingNode* fn )
{

}

