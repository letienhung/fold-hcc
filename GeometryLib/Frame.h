#pragma once

#include "UtilityGlobal.h"

namespace Goem{

struct Frame
{
	Vector3 c;
	Vector3 r, s, t;

	Frame();
	Frame(const Vector3& C, const Vector3& R, const Vector3& S, const Vector3& T);

	void	normalize();
	Vector3	coordinates(const Vector3& p);
	Vector3 position(const Vector3& coord); 

	bool	isAligned(const Frame& other);
};

}