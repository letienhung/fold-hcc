#pragma once

#include "UtilityGlobal.h"

namespace Geom{

struct Frame
{
	Vector3 c;
	Vector3 r, s, t;

	Frame();
	Frame(const Vector3& C, const Vector3& R, const Vector3& S);
	Frame(const Vector3& C, const Vector3& R, const Vector3& S, const Vector3& T);

	Vector3	getCoords(Vector3 p);
	Vector3 getPosition(Vector3 coord); 

	bool	isAlignedWith(const Frame& other);

	// encode and decode
	struct RecordInFrame{
		Vector3 c, cpr, cps, cpt;
	};
	RecordInFrame encodeFrame(Frame& other);
	Frame decodeFrame(RecordInFrame record);

};

}