#pragma once

#include "Node.h"

#include <Eigen/core>
#include <Eigen/Dense>  
#include <Eigen/Eigen>

enum DEFORM_MODE{SELECT, SCALEMODE, TRANSMODE, ROTATEMODE};

class Deformer
{
public:
	Deformer(Node *n);
	~Deformer();

private:
	// Factor for deformation
	Vector3 mFactor;
	// Rotation Matrix
	Eigen::Matrix3f mMat;
	qglviewer::Quaternion mQuan;

	Node *mNode;

	void scale();
	void rotate();
	void translate();

public:
	void updateFactor(Vector3 &vec);
	void updateRotMat(Eigen::Matrix3f &m);
	void updateRotQuan(qglviewer::Quaternion &q);
	void Deform(DEFORM_MODE mode);
	void updateNode(Node *n);
};