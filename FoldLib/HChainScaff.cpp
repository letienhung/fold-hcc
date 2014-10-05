#include "HChainScaff.h"
#include "Numeric.h"

HChainScaff::HChainScaff(ScaffNode* slave, PatchNode* base, PatchNode* top)
	:ChainScaff(slave, base, top)
{
	// uniform
	useUniformHeight = true;
}

Geom::Rectangle HChainScaff::getFoldRegion(FoldOption* fn)
{
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Segment topJointProj = base_rect.getProjection(topJoint);

	double d = rightSeg.length();
	double l = slaveSeg.length();
	double offset = (l - d) / (fn->nSplits + 1);

	Geom::Segment rightSeg, leftSeg;
	if (fn->rightSide)
	{
		leftSeg = topJointProj;
		rightSeg = baseJoint.translated(offset * rightDirect);
	}
	else
	{
		leftSeg = topJointProj.translated(-offset * rightDirect);
		rightSeg = baseJoint;
	}

	// shrink along jointV
	double t0 = fn->position;
	double t1 = t0 + fn->scale;
	leftSeg.cropRange01(t0, t1);
	rightSeg.cropRange01(t0, t1);

	// fold region in 3D
	Geom::Rectangle region(QVector<Vector3>()
		<< leftSeg.P0 << leftSeg.P1
		<< rightSeg.P1 << rightSeg.P0);

	return region;
}

//     left					right
//	   ---						|
//		|step					|
//	   ---						|
//		|step					|d
//	  -----	plane0				|
//		|						|
//		|					  -----
//		|						|step
//		|d					   ---
//		|						|step
//		|					   --- plane0
QVector<Geom::Plane> HChainScaff::generateCutPlanes(FoldOption* fn)
{
	// constants
	double L = slaveSeg.length();
	double d = rightSeg.length();
	double h = upSeg.length();
	double a = (L - d) / (fn->nSplits + 1);
	double sin_alpha = h / L;

	// start plane and step
	double step = a * sin_alpha;
	Geom::Plane start_plane;
	Vector3 stepV;
	{// right: cut at lower end
		start_plane = baseMaster->mPatch.getPlane();
		stepV = step * start_plane.Normal;
	}
	if (!fn->rightSide)
	{// left: cut at higher end
		start_plane.translate(upSeg.P1 - upSeg.P0);
		stepV *= -1;
	}

	// cut planes
	QVector<Geom::Plane> cutPlanes;
	for (int i = 1; i <= fn->nSplits; i++)
	{
		Vector3 deltaV = i * stepV;
		cutPlanes << start_plane.translated(deltaV);
	}

	return cutPlanes;
}

void HChainScaff::fold(double t)
{
	// free all nodes
	for(Structure::Node* n : nodes)
		n->removeTag(FIXED_NODE_TAG);

	// fix base
	baseMaster->addTag(FIXED_NODE_TAG);

	// set up hinge angles and top position
	if (useUniformHeight)
		foldUniformHeight(t);
	else
		foldUniformAngle(t);

	// restore configuration
	restoreConfiguration();
}

void HChainScaff::foldUniformAngle(double t)
{
	// hinge angles
	double d = rightSeg.length();
	double sl = slaveSeg.length();
	int n = chainParts.size();
	double alpha = acos(RANGED(0, d / sl, 1)) * (1 - t);

	double a = (sl - d) / n;
	double b = d + a;
	double bProj = b * cos(alpha);
	double beta;

	// phase separator
	computePhaseSeparator();

	// no return
	if (bProj <= d)
	{
		double cos_beta = (d - bProj) / ((n - 1) * a);
		beta = acos(RANGED(0, cos_beta, 1));

		if (foldToRight)
		{
			activeLinks[0]->hinge->angle = M_PI - beta;
			for (int i = 1; i < activeLinks.size() - 1; i++)
				activeLinks[i]->hinge->angle = M_PI;
			activeLinks.last()->hinge->angle = alpha + M_PI - beta;
		}
		else
		{
			activeLinks[0]->hinge->angle = alpha;
			activeLinks[1]->hinge->angle = alpha + M_PI - beta;
			for (int i = 2; i < activeLinks.size(); i++)
				activeLinks[i]->hinge->angle = M_PI;
		}
	}
	// return
	else
	{
		double cos_beta = (b * cos(alpha) - d) / a;
		beta = acos(RANGED(0, cos_beta, 1));

		if (!activeLinks.size()) return;

		if (foldToRight)
		{
			activeLinks[0]->hinge->angle = beta;
			for (int i = 1; i < activeLinks.size() - 1; i++)
				activeLinks[i]->hinge->angle = 2 * beta;
			activeLinks.last()->hinge->angle = alpha + beta;
		}
		else
		{
			activeLinks[0]->hinge->angle = alpha;
			activeLinks[1]->hinge->angle = alpha + beta;
			for (int i = 2; i < activeLinks.size(); i++)
				activeLinks[i]->hinge->angle = 2 * beta;
		}
	}

	// adjust the position of top master
	double ha = a * sin(beta);
	double hb = b * sin(alpha);
	double topH = (n - 1) * ha + hb;
	Vector3 topPos = upSeg.P0 + topH * upSeg.Direction;
	topMaster->translate(topPos - topMaster->center());
}

void HChainScaff::foldUniformHeight(double t)
{
	// constant
	int n = chainParts.size();
	double L = slaveSeg.length();
	double d = rightSeg.length();
	double a = (L - d) / n;
	double b = d + a;
	double A = (n - 1) * a;
	double H = upSeg.length();
	double h = (1 - t) * H;

	// phase separator
	computePhaseSeparator();

	// phase-I: no return
	if (h >= heightSep)
	{
		double alpha_orig = acos(RANGED(0, d / L, 1));
		TimeInterval alpha_it(angleSep, alpha_orig);
		double x = 2 * b * h;
		double y = 2 * b * d;
		double z = d * d + h * h + b * b - A * A;
		double aa = x * x + y * y;
		double bb = -2 * y * z;
		double cc = z * z - x * x;

		double alpha = 0;
		QVector<double> roots = findRoots(aa, bb, cc);
		foreach(double r, roots){
			alpha = acos(RANGED(0, r, 1));
			if (alpha_it.contains(alpha)) break;
		}
		if (alpha == 0) alpha = alpha_orig; //set original angle if no roots found

		double cos_beta = (d - b * cos(alpha)) / A;
		double beta = acos(RANGED(0, cos_beta, 1));

		// set angles
		if (foldToRight)
		{
			activeLinks[0]->hinge->angle = M_PI - beta;
			for (int i = 1; i < activeLinks.size() - 1; i++)
				activeLinks[i]->hinge->angle = M_PI;
			activeLinks.last()->hinge->angle = alpha + M_PI - beta;
		}
		else
		{
			activeLinks[0]->hinge->angle = alpha;
			activeLinks[1]->hinge->angle = alpha + M_PI - beta;
			for (int i = 2; i < activeLinks.size(); i++)
				activeLinks[i]->hinge->angle = M_PI;
		}
	}
	// phase-II: return
	else
	{
		TimeInterval alpha_it(0, angleSep);
		double B = b * b * n * (n - 2);
		double C = -2 * b * d * (n - 1) * (n - 1);
		double D = A * A - h * h - b * b - (n - 1) * (n - 1) * d * d;
		double E = 4 * b * b * h * h - 2 * B * D + C * C;
		double F = -2 * C * D;
		double G = D * D - 4 * b * b * h * h;
		double K = 2 * B * C;

		double alpha = 0, beta = 0;
		QVector<double> roots = findRoots(B * B, K, E, F, G);
		foreach(double r, roots)
		{
			if (r < 0) continue;

			alpha = acos(RANGED(0, r, 1));
			if (alpha_it.contains(alpha))
			{
				double cos_beta = (b * r - d) / a;
				beta = acos(RANGED(0, cos_beta, 1));

				double hh = A * sin(beta) + b * sin(alpha);
				if (fabs(h - hh) < ZERO_TOLERANCE_LOW)
					break;
			}
		}
		// angles are sensitive to noise at very beginning or end
		if (t < 0.01)
		{
			alpha = angleSep;
			double cos_beta = (b * cos(alpha) - d) / a;
			beta = acos(RANGED(0, cos_beta, 1));
		}
		if (1 - t < 0.01)
		{
			alpha = 0;
			double cos_beta = (b * cos(alpha) - d) / a;
			beta = acos(RANGED(0, cos_beta, 1));
		}

		// set angles
		if (foldToRight)
		{
			activeLinks[0]->hinge->angle = beta;
			for (int i = 1; i < activeLinks.size() - 1; i++)
				activeLinks[i]->hinge->angle = 2 * beta;
			activeLinks.last()->hinge->angle = alpha + beta;
		}
		else
		{
			activeLinks[0]->hinge->angle = alpha;
			activeLinks[1]->hinge->angle = alpha + beta;
			for (int i = 2; i < activeLinks.size(); i++)
				activeLinks[i]->hinge->angle = 2 * beta;
		}
	}

	// adjust the position of top master
	Vector3 topPos = upSeg.P0 + h * upSeg.Direction;
	topMaster->translate(topPos - topMaster->center());
}

void HChainScaff::computePhaseSeparator()
{
	// constant
	int n = chainParts.size();
	double L = slaveSeg.length();
	double d = rightSeg.length();
	double a = (L - d) / n;
	double b = d + a;
	double A = (n - 1) * a;

	// height
	heightSep = A + sqrt(b * b - d * d);

	// angle
	double sin_angle = heightSep - A;
	angleSep = asin(RANGED(0, sin_angle, 1));
}

QVector<FoldOption*> HChainScaff::genRegularFoldOptions(int maxNbSplits, int maxNbChunks)
{
	// enumerate all possible combination of nS and nC
	QVector<FoldOption*> options;
	for (int nS = 1; nS <= maxNbSplits; nS += 2)
	for (int nC = 1; nC <= maxNbChunks; nC++)
		options << ChainScaff::genRegularFoldOptions(nS, nC, maxNbChunks);

	return options;
}

