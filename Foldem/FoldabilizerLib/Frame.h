#pragma once

#include "FoldabilizerLibGlobal.h"

struct Frame
{
	Vec3d c;
	Vec3d r, s, t;

	Frame();
	Frame(const Vec3d& C, const Vec3d& R, const Vec3d& S, const Vec3d& T);

	void normalize();
	Vec3d coordinates(const Vec3d& p);
	Vec3d position(const Vec3d& coord); 
};

