load($$[STARLAB])
load($$[SURFACEMESH])
StarlabTemplate(none)

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
    Polygon.h

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
    Polygon.cpp
