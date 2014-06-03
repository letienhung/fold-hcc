#pragma once

#include "PropertyMap.h"

namespace Structure{

class Node : public PropertyMap
{
public:
	Node(QString nid); 
	virtual ~Node();
	Node(Node &other);
	virtual Node* clone();

	// identity
	bool hasId(QString id);

	// selection
	void flipSelect();

	// visualization
	virtual void draw();
	virtual void drawWithName(int name);

public:
	QString	mID;
	bool isSelected;
};
}
