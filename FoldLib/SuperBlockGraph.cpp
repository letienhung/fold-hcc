#include "SuperBlockGraph.h"
#include "HBlockGraph.h"
#include "ShapeSuperKeyframe.h"
#include "ChainGraph.h"
#include "Numeric.h"

SuperBlockGraph::SuperBlockGraph(HBlockGraph* block, ShapeSuperKeyframe* sskf)
: origBlock(block), ssKeyframe(sskf)
{
	// map from master to super master within this block
	// self mapping if no corresponding super master
	QMap<QString, QString> master2Super;

	// clone parts from block
	foreach(Structure::Node* n, origBlock->nodes)
	{
		Structure::Node* clone_n;

		// super patch (master)
		if (ssKeyframe->master2SuperMap.contains(n->mID))
		{
			Structure::Node* super_n = ssKeyframe->getNode(ssKeyframe->master2SuperMap[n->mID]);
			clone_n = super_n->clone();

			// copy mapping
			master2Super[n->mID] = ssKeyframe->master2SuperMap[n->mID];
		}
		// regular node
		else
		{
			clone_n = n->clone();

			// self mapping
			master2Super[n->mID] = n->mID;
		}

		// add to super block
		Structure::Graph::addNode(clone_n);
	}

	// the base master
	QString base_mid = master2Super[origBlock->baseMaster->mID];
	baseMaster = (PatchNode*)getNode(base_mid);

	// other master related stuff
	foreach(PatchNode* m, origBlock->masters)
	{
		QString mid = m->mID;
		QString mid_super = master2Super[mid];

		// super masters
		masters << (PatchNode*)getNode(mid_super);

		// master height
		masterHeight[mid_super] = origBlock->masterHeight[mid];

		// maps between chain and top masters
		QSet<int> underChainIndices = origBlock->masterUnderChainsMap[mid];
		masterUnderChainsMap[mid_super] = underChainIndices;
		foreach(int ucid, underChainIndices)
			chainTopMasterMap[ucid] = mid_super;
	}

}

void SuperBlockGraph::computeMinFoldingRegion()
{
	minFoldingRegion.clear();
	Geom::Rectangle base_rect = origBlock->baseMaster->mPatch; // ***use original base rect
	foreach(PatchNode* top_master, masters)
	{
		// skip base master
		if (top_master == baseMaster) continue;

		// min folding region is the projection of top master
		Geom::Rectangle2 min_region = base_rect.get2DRectangle(top_master->mPatch);

		// store
		minFoldingRegion[top_master->mID] = min_region;

		// debug
		Geom::Rectangle min_region3 = base_rect.get3DRectangle(min_region);
		//origBlock->properties[MINFR].setValue(min_region3.getEdgeSamples(20));
	}
}

void SuperBlockGraph::computeMaxFoldingRegion()
{
	maxFoldingRegion.clear();
	Geom::Rectangle base_rect = origBlock->baseMaster->mPatch;// ***use original base rect
	int aid = origBlock->shapeAABB.getAxisId(base_rect.Normal);
	Geom::Rectangle cropper3 = origBlock->shapeAABB.getPatch(aid, 0);
	Geom::Rectangle2 cropper2 = base_rect.get2DRectangle(cropper3);
	foreach(PatchNode* top_master, masters)
	{
		// skip base master
		if (top_master == baseMaster) continue;

		// projection of folded slaves with minimum splits
		QVector<Vector2> pnts_proj;
		QVector<Vector3> pnts;
		foreach(int cid, masterUnderChainsMap[top_master->mID]) {
			pnts << origBlock->chains[cid]->getFoldRegion(true, 0).getConners();
			pnts << origBlock->chains[cid]->getFoldRegion(false, 0).getConners();
		}
		foreach(Vector3 p, pnts) pnts_proj << base_rect.getProjCoordinates(p);

		// projection of top master
		pnts_proj << minFoldingRegion[top_master->mID].getConners();

		// max region is the AABB of projections of both folded slave and top master
		Geom::Rectangle2 max_region = computeAABB2D(pnts_proj);

		// max region is confined by the bounding box
		max_region.cropByAxisAlignedRectangle(cropper2);

		// store
		maxFoldingRegion[top_master->mID] = max_region;

		// debug
		QVector<Vector3> pnts_proj3;
		foreach (Vector2 p2, pnts_proj) 
			pnts_proj3 << base_rect.getPosition(p2);
		//origBlock->properties[MAXFR].setValue(pnts_proj3);

		//origBlock->properties[MAXFR].setValue(cropper3.getEdgeSamples(70));

		Geom::Rectangle max_region3 = base_rect.get3DRectangle(max_region);
		origBlock->properties[MAXFR].setValue(max_region3.getEdgeSamples(100));
	}
}

bool SuperBlockGraph::computeAvailFoldingRegion()
{
	// clear
	availFoldingRegion.clear();

	// align super shape key frame with this super block
	Vector3 pos_block = baseMaster->center();
	FdNode* fnode = (FdNode*)ssKeyframe->getNode(baseMaster->mID);
	if (!fnode) return false;
	Vector3 pos_keyframe = fnode->center();
	Vector3 offset = pos_block - pos_keyframe;
	ssKeyframe->translate(offset, false);

	// min/max folding regions
	computeMinFoldingRegion();
	computeMaxFoldingRegion();

	// extend min until max
	QString base_mid = baseMaster->mID;
	Geom::Rectangle base_rect = origBlock->baseMaster->mPatch;// ***use original base rect
	foreach(PatchNode* top_master, masters)
	{
		// skip base master
		QString top_mid = top_master->mID;
		if (top_mid == base_mid) continue;

		// samples from constraint parts: in-between and unordered
		QVector<QString> constraintParts;
		constraintParts << origBlock->getInbetweenExternalParts(baseMaster->center(), top_master->center(), ssKeyframe);
		constraintParts << getUnrelatedMasters(base_mid, top_mid);
		QVector<Vector3> samples;
		QVector<Vector2> samples_proj;
		for (auto nid : constraintParts)
			samples << ssKeyframe->getFdNode(nid)->sampleBoundabyOfScaffold(100);
		for (auto s : samples) samples_proj << base_rect.getProjCoordinates(s);

		// sample boundary of max region
		samples_proj << maxFoldingRegion[top_mid].getEdgeSamples(100);

		// avail folding region
		// includes minFR but excludes all constraint points
		Geom::Rectangle2 avail_region = minFoldingRegion[top_mid];
		bool okay = extendRectangle2D(avail_region, samples_proj);
		if (!okay) return false; // availFR < minFR
		availFoldingRegion[top_mid] = avail_region;

		// debug
		QVector<Vector3> samples_proj3;
		foreach(Vector2 p2, samples_proj)
			samples_proj3 << base_rect.getPosition(p2);
		//origBlock->properties[AFR_CP].setValue(samples_proj3);

		Geom::Rectangle avail_region3 = base_rect.get3DRectangle(avail_region);
		origBlock->properties[AFR_CP].setValue(avail_region3.getEdgeSamples(70));
	}

	// restore the position of shape super key frame
	ssKeyframe->translate(-offset, false);
	return true;
}

QVector<QString> SuperBlockGraph::getUnrelatedMasters(QString base_mid, QString top_mid)
{
	QVector<QString> urMasters;

	// retrieve master order constraints
	StringSetMap moc_greater = ssKeyframe->mocGreater;
	StringSetMap moc_less = ssKeyframe->mocLess;

	// related parts with mid1 and mid2
	QSet<QString> base_moc = moc_greater[base_mid] + moc_less[base_mid];
	QSet<QString> top_moc = moc_greater[top_mid] + moc_less[top_mid];

	// masters unrelated with both
	foreach(PatchNode* m, getAllMasters(ssKeyframe))
	{
		// skip folded masters
		if (m->hasTag(MERGED_PART_TAG)) continue;

		// accept if not related to any
		if (!base_moc.contains(m->mID) && !top_moc.contains(m->mID))
			urMasters << m->mID;
	}

	return urMasters;
}

double SuperBlockGraph::getAvailFoldingVolume()
{
	Geom::Rectangle base_rect = origBlock->baseMaster->mPatch;
	QList<QString> super_mids = availFoldingRegion.keys();

	// trivial case with single box
	if (availFoldingRegion.size() == 1)
	{
		QString smid = super_mids.front();
		Geom::Rectangle afr3 = base_rect.get3DRectangle(availFoldingRegion[smid]);
		return afr3.area() * masterHeight[smid];
	}

	// AABB of avail folding regions
	QVector<Vector2> conners;
	foreach(QString key, availFoldingRegion.keys())
		conners << availFoldingRegion[key].getConners();
	Geom::Rectangle2 aabb2 = computeAABB2D(conners);

	if (!_finite(aabb2.Extent.x()))
		return 0;

	// pixelize folding region
	Geom::Rectangle aabb3 = base_rect.get3DRectangle(aabb2);
	double pixel_size = aabb3.Extent[0] / 100;
	double pixel_area = pixel_size * pixel_size;
	double AFV = 0;
	foreach(Vector3 gp3, aabb3.getGridSamples(pixel_size))
	{
		Vector2 gp2 = base_rect.getProjCoordinates(gp3);

		double best_height = 0;
		foreach(QString smid, super_mids)
		if (availFoldingRegion[smid].contains(gp2) && masterHeight[smid] > best_height)
			best_height = masterHeight[smid];

		AFV += best_height;
	}
	AFV *= pixel_area;

	return AFV;
}

Geom::Box SuperBlockGraph::getAFS(QString mid)
{
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Rectangle afr3 = base_rect.get3DRectangle(availFoldingRegion[mid]);
	Geom::Box afs(afr3, base_rect.Normal, masterHeight[mid]);

	double epsilon = 2 * ZERO_TOLERANCE_LOW;
	afs.Extent += Vector3(epsilon, epsilon, epsilon);

	return afs;
}

QVector<Geom::Box> SuperBlockGraph::getAllAFS()
{
	QVector<Geom::Box> afs;
	foreach(QString top_mid, availFoldingRegion.keys())
		afs << getAFS(top_mid);
	return afs;
}

QVector<Geom::Rectangle2> SuperBlockGraph::getAvailFoldingRegion()
{
	return availFoldingRegion.values().toVector();
}
