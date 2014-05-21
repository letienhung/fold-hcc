#pragma once
#include <QString>
#include <QVariant>
#include <QMap>
#include <QVector>

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

	template<T> getProperty(QString key)
	{
		return properties[key].value<T>();
	}

	template<class C, class T>
	void appendToContainerProperty(QString key, C<T> vec){
		C<T> vprop;
		if (properties.contains(key))
			vprop = properties[key].value<C<T> >();
		vprop << vec;
		properties[key].setValue(vprop);
	}

	template<class C, class T>
	void appendToContainerProperty(QString key, T value){
		appendToContainerProperty(key, C<T>() << value);
	}

public:
	QMap<QString, QVariant> properties;
};

}