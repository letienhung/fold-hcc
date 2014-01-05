#pragma once

#include "UtilityGlobal.h"
#include "Box.h"

namespace Geom{

	class PcaOBB
	{
	public:
		PcaOBB(QVector<Vector3>& pnts);

		Box minBox;
	};

}
