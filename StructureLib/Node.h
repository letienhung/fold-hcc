#pragma once

#include <QString>
#include <QVariant>
#include <QMap>

typedef QMap< QString, QVariant > PropertyMap;

namespace Structure{

class Node
{
public:
	Node(QString nid); 
	Node(Node &other);
	virtual Node* clone();

	// identity
	bool hasId(QString id);

	// selection
	void flipSelect();

	// visualization
	virtual void draw();
	virtual void drawWithName(int name);

	// properties
	void addTag(QString tag);
	bool hasTag(QString tag);

public:
	QString	mID;
	bool isSelected;
    PropertyMap properties;
};
}
