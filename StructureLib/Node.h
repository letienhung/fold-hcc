#pragma once

#include "PropertyContainer.h"

namespace Structure{

class Node : public PropertyContainer
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

public:
	QString	mID;
	bool isSelected;
};
}
