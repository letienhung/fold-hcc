#include "TransformationPanel.h"

TranslationPanel::TranslationPanel()
{
	transWidget.setupUi(this);
    
	connect(transWidget.TranslateButton, SIGNAL(clicked()), SLOT(applyTranslation()));
}

void TranslationPanel::updateNode(Node* n)
{
	mNode = n;
}

RotationPanel::RotationPanel()
{
   rotWidget.setupUi(this);

   connect(rotWidget.rotateButton, SIGNAL(clicked()), SLOT(applyRotation()));
}

void RotationPanel::updateNode(Node *n)
{
	mNode = n;
}

ScalePanel::ScalePanel()
{
   scaleWidget.setupUi(this);

   connect(scaleWidget.ScaleButton, SIGNAL(clicked()), SLOT(applyScale()));
}

void ScalePanel::updateNode(Node *n)
{
	mNode = n;
}

void RotationPanel::applyRotation()
{
	double xAngle = RADIANS(rotWidget.xAngle->value());
	double yAngle = RADIANS(rotWidget.yAngle->value());
	double zAngle = RADIANS(rotWidget.zAngle->value());

	qglviewer::Quaternion qx(qglviewer::Vec(1,0,0), xAngle);
	qglviewer::Quaternion qy(qglviewer::Vec(0,1,0), yAngle);
	qglviewer::Quaternion qz(qglviewer::Vec(0,0,1), zAngle);

	mQuan = qx * qy * qz;
	mNode->rotate(mQuan);
}

void ScalePanel::applyScale()
{
	mScale = Vector3(scaleWidget.xAngle->value(), scaleWidget.yAngle->value(), scaleWidget.zAngle->value());
	mNode->scaleFactor = mScale;
	mNode->mBox.Extent = Vector3(mNode->mBox.Extent[0]*mNode->scaleFactor[0], 
								 mNode->mBox.Extent[1]*mNode->scaleFactor[1],
								 mNode->mBox.Extent[2]*mNode->scaleFactor[2]);

}

void TranslationPanel::applyTranslation()
{
	mTrans = Vector3(transWidget.xAngle->value(), transWidget.yAngle->value(), transWidget.zAngle->value());
	mNode->translate(mTrans);

}
