#pragma once
#include <QString>
#include <QVariant>
#include <QMap>
#include <QVector>
#include <QSet>

Q_DECLARE_METATYPE(QVector<QString>)


namespace Structure
{

class PropertyMap
{
public:
    PropertyMap(){}

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

	bool containsProperty(QString key)
	{
		return properties.contains(key);
	}

	template<class T> 
	T getProperty(QString key)
	{
		return properties[key].value<T>();
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

	template<class T>
	void appendToSetProperty(QString key, QSet<T> vec){
		QSet<T> vprop;
		if (properties.contains(key))
			vprop = properties[key].value<QSet<T> >();
		vprop << vec;
		properties[key].setValue(vprop);
	}

	template<class T>
	void appendToSetProperty(QString key, T value){
		appendToSetProperty(key, QSet<T>() << value);
	}

public:
	QMap<QString, QVariant> properties;
};

}