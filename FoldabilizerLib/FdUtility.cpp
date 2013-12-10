#include "FdUtility.h"

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