#include "SurfaceMeshHelper.h"
#include "qglviewer/manipulatedFrame.h"
#include "Box.h"
#include "Deformer.h"

using namespace qglviewer;
using namespace SurfaceMesh;

class DeformHandle: public ManipulatedFrame{
	Q_OBJECT

public:
	DeformHandle(const Vector3 & start){
		updateStart(start);
	}

	Vector3 transform(const Vector3 &base){
		Vec d = this->position() - Vec(startPos.x(), startPos.y(),startPos.z());
		return Vector3(d[0] / base.x(), d[1] / base.y(), d[2] / base.z());
	}

	/* Eigen::Matrix3f rotate()
	{
	Quaternion q = this->rotation();

	float matrix[3][3];
	q.getRotationMatrix(matrix);

	Eigen::Matrix3f m;
	for(int i = 0; i < 3; i++)
	for(int j = 0; j < 3; j++)
	m(i,j) = matrix[i][j];  

	return m;
	}*/

	Quaternion rotate()
	{
		return this->rotation();
	}

	void updateStart(const Vector3 & start)
	{
		this->startPos = start;
		this->setPosition(start.x(), start.y(), start.z());
	}
private:
	Vector3 startPos;
};
