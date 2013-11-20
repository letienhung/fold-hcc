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

# Geometry library
LIBS += -L$$PWD/../GeometryLib/$$CFG/lib -lGeometryLib
INCLUDEPATH += ../GeometryLib

# Library name and destination
TARGET = FoldabilizerLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    Graph.h \
    Node.h \
    Link.h \
    FoldabilizerLibGlobal.h \
    xmlWriter.h \
    MHOptimizer.h \
    HingeDetector.h \
    Hinge.h \
    ProbabilityDistributions.h

SOURCES += \
    Graph.cpp \
    Node.cpp \
    Link.cpp \
    xmlWriter.cpp \
    MHOptimizer.cpp \
    HingeDetector.cpp \
    Hinge.cpp
