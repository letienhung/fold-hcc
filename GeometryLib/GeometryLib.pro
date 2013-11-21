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
    IntersectBoxBox.h \
    Frame.h \
    Box.h \
    Rectangle.h

SOURCES += \
    Segment.cpp \
    Plane.cpp \
    Line.cpp \
    IntersectBoxBox.cpp \
    Frame.cpp \
    Box.cpp \
    Rectangle.cpp
