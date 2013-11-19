#pragma once
#include "FoldabilizerLibGlobal.h"
#include "Box.h"
#include "Hinge.h"

class HingeDetector
{
public:
    HingeDetector();

	static QVector<Hinge> getHinges(Box &box0, Box &box1);
};
