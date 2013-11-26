#pragma once

#include "ui_RotationWidget.h"
#include "ui_ScaleWidget.h"
#include "ui_TranslationWidget.h"

#include "Node.h"

#define RADIANS(deg)    ((deg)/180.0 * M_PI)

class TranslationPanel : public QWidget
{
	Q_OBJECT

public:
	TranslationPanel();

	Ui::TranslationWidget transWidget;

	Vector3 mTrans;
	Node *mNode;
	void updateNode(Node *n);

public slots:
	void applyTranslation();
};

class RotationPanel : public QWidget
{
	Q_OBJECT

public:
	RotationPanel();

	Ui::RotationWidget rotWidget;

	qglviewer::Quaternion mQuan;
	Node *mNode;
	void updateNode(Node *n);

	public slots:
		void applyRotation();
};

class ScalePanel : public QWidget
{
	Q_OBJECT

public:
	ScalePanel();

	Ui::ScaleWidget scaleWidget;

	Vector3 mScale;
	Node *mNode;
	void updateNode(Node *n);
	
	public slots:
		void applyScale();
};
