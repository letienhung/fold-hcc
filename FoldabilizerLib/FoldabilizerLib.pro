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
TARGET = FoldabilizerLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    Graph.h \
    Node.h \
    Link.h \
    FoldabilizerLibGlobal.h \
    Box.h \
    IntersectBoxBox.h \
    Frame.h \
    xmlWriter.h \
    Numeric.h \
    MHOptimizer.h \
    Plane.h

SOURCES += \
    Graph.cpp \
    Node.cpp \
    Link.cpp \
    Box.cpp \
    IntersectBoxBox.cpp \
    Frame.cpp \
    xmlWriter.cpp \
    MHOptimizer.cpp \
    Plane.cpp
