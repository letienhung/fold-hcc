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

# Geometry library
LIBS += -L$$PWD/../GeometryLib/$$CFG/lib -lGeometryLib
INCLUDEPATH += ../GeometryLib

# Library name and destination
TARGET = FoldabilizerLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    Node.h \
    Link.h \
    MHOptimizer.h \
    HingeDetector.h \
    Hinge.h \
    HccGraph.h \
    HccManager.h \
    NodeSplitter.h

SOURCES += \
    Node.cpp \
    Link.cpp \
    MHOptimizer.cpp \
    HingeDetector.cpp \
    Hinge.cpp \
    HccGraph.cpp \
    HccManager.cpp \
    NodeSplitter.cpp
