#pragma once
#include <QString>
#include <QVariant>
#include <QMap>
#include <QVector>

Q_DECLARE_METATYPE(QVector<QString>)


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

	template<class T>
	void addProperty(QString key, T value)
	{
		properties[k] = value;
	}

	template<class T>
	void appendToVectorProperty(QString key, QVector<T> vec){
		QVector<T> vprop;
		if (properties.contains(key))
			vprop = properties[key].value<QVector<T> >();
		vprop << vec;
		properties[key].setValue(vprop);
	}

	template<class T>
	void appendToVectorProperty(QString key, T value){
		appendToVectorProperty(key, QVector<T>() << value);
	}

public:
	PropertyMap properties;
};

}