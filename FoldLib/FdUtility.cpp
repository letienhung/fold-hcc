#include "FdUtility.h"
#include "UtilityGlobal.h"

void writeBoxToXml( XmlWriter& xw, Geom::Box& b )
{
	xw.writeOpenTag("box");
	{
		xw.writeTaggedString("c", qStr(b.Center));
		xw.writeTaggedString("x", qStr(b.Axis[0]));
		xw.writeTaggedString("y", qStr(b.Axis[1]));
		xw.writeTaggedString("z", qStr(b.Axis[2]));
		xw.writeTaggedString("e", qStr(b.Extent));
	}
	xw.writeCloseTag("box");
}

Geom::Box getBox( QDomNode& node )
{
	QString c = node.firstChildElement("c").text();
	QString x = node.firstChildElement("x").text();
	QString y = node.firstChildElement("y").text();
	QString z = node.firstChildElement("z").text();
	QString e = node.firstChildElement("e").text();

	Geom::Box box;
	box.Center = toVector3(c);
	box.Axis.clear();
	box.Axis << toVector3(x) << toVector3(y) << toVector3(z);
	box.Extent = toVector3(e);

	return box;
}
