#pragma once
#include <QString>
#include <QVariant>
#include <QMap>

namespace Structure
{

typedef QMap< QString, QVariant > PropertyMap;

class PropertyContainer
{
public:
    PropertyContainer(){}

	// properties
	void addTag(QString tag){
		properties[tag] = true;
	}

	void removeTag(QString tag){
		properties.remove(tag);
	}

	bool hasTag(QString tag){
		return properties.contains(tag);
	}

public:
	PropertyMap properties;
};

}