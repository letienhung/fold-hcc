#pragma once

#include <QString>

namespace Structure{

class Node
{
public:
	Node(QString id) : id(id){
		isSelected = false;
	}
	~Node(){}

	bool hasId(QString id){ return this->id == id;}
	void select(){isSelected = !isSelected;}
	virtual void draw(){}

public:
	QString	id;
	bool isSelected;
};                                 

}