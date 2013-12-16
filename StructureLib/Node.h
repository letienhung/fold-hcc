#pragma once

#include <QString>


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

public:
	QString	id;
	bool isSelected;
};                                 

}