include($$[STARLAB])
include($$[SURFACEMESH])


TEMPLATE = lib
CONFIG += staticlib

# Build flag
CONFIG(debug, debug|release) {
    CFG = debug
} else {
    CFG = release
}

# Utility library
LIBS += -L$$PWD/../UtilityLib/$$CFG/lib -lUtilityLib
INCLUDEPATH += ../UtilityLib

# Library name and destination
TARGET = GeometryLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    Segment.h \
    Plane.h \
    Line.h \
    Frame.h \
    Box.h \
    Rectangle.h \
    Segment2.h \
    IntrBoxBox.h \
    IntrSeg2Seg2.h \
    IntrRectRect.h \
    Polygon.h \
    AABB.h \
    ConvexHull2.h \
    MinOBB2.h \
    MinOBB.h \
    PcaOBB.h \
    Circle.h\
    ConvexHull.h \
    DistLineLine.h \
    DistSegSeg.h \
    DistSegRect.h \
    DistLineRect.h \
    DistLineSeg.h \
    DistPointRect.h \
    DistRectRect.h \
    SectorCylinder.h \
    Rectangle2.h \
    IntrRect2Rect2.h \
    PCA.h \
    GeomUtility.h \
    Frame2.h

SOURCES += \
    Segment.cpp \
    Plane.cpp \
    Line.cpp \
    Frame.cpp \
    Box.cpp \
    Rectangle.cpp \
    Segment2.cpp \
    IntrBoxBox.cpp \
    IntrSeg2Seg2.cpp \
    IntrRectRect.cpp \
    Polygon.cpp \
    AABB.cpp \
    ConvexHull2.cpp \
    MinOBB2.cpp \
    MinOBB.cpp \
    Circle.cpp\
    ConvexHull.cpp \
    DistLineLine.cpp \
    DistSegSeg.cpp \
    DistSegRect.cpp \
    DistLineRect.cpp \
    DistLineSeg.cpp \
    DistPointRect.cpp \
    DistRectRect.cpp \
    SectorCylinder.cpp \
    Rectangle2.cpp \
    IntrRect2Rect2.cpp \
    PCA.cpp \
    PcaOBB.cpp \
    GeomUtility.cpp \
    Frame2.cpp
