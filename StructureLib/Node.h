#pragma once

#include "Link.h"

namespace Structure{

class Node
{
public:
	Node(QString id) : id(id){}

	bool hasId(QString id){ return this->id == id;}
	virtual void draw(){}

public:
	QString	id;
};                                 

}