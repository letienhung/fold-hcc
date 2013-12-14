#pragma once

#include "XmlWriter.h"
#include "Box.h"
#include <QDomNode>

void writeBoxToXml( XmlWriter& xw, Geom::Box& b );
Geom::Box getBox(QDomNode& node);