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

# Library name and destination
TARGET = GeometryLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    Segment.h \
    Rectangle.h \
    Plane.h \
    Line.h \
    IntersectBoxBox.h \
    Frame.h \
    Box.h \
    GeometryLibGlobal.h

SOURCES += \
    Segment.cpp \
    Rectangle.cpp \
    Plane.cpp \
    Line.cpp \
    IntersectBoxBox.cpp \
    Frame.cpp \
    Box.cpp
