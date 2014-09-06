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

# Geometry library
LIBS += -L$$PWD/../GeometryLib/$$CFG/lib -lGeometryLib
INCLUDEPATH += ../GeometryLib

# Structure library
LIBS += -L$$PWD/../StructureLib/$$CFG/lib -lStructureLib
INCLUDEPATH += ../StructureLib

# Mesh Utility library
LIBS += -L$$PWD/../MeshUtilityLib/$$CFG/lib -lMeshUtilityLib
INCLUDEPATH += ../MeshUtilityLib

# Cliquer library
LIBS += -L$$PWD/../CliquerLib/$$CFG/lib -lCliquerLib
INCLUDEPATH += ../CliquerLib

# Library name and destination
TARGET = FoldLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    PatchNode.h \
    RodNode.h \
    GraphManager.h \
    FdGraph.h \
    FdLink.h \
    FdNode.h \
    DcGraph.h \
    FoldManager.h \
    FdUtility.h \
    BlockGraph.h \
    ChainGraph.h \
    Hinge.h \
    BundleNode.h \
    FoldOptionGraph.h \
    ShapeSuperKeyframe.h \
    SuperBlockGraph.h \
    TChainGraph.h \
    TBlockGraph.h

SOURCES += \
    PatchNode.cpp \
    RodNode.cpp \
    GraphManager.cpp \
    FdGraph.cpp \
    FdLink.cpp \
    FdNode.cpp \
    DcGraph.cpp \
    FoldManager.cpp \
    FdUtility.cpp \
    BlockGraph.cpp \
    ChainGraph.cpp \
    Hinge.cpp \
    BundleNode.cpp \
    FoldOptionGraph.cpp \
    ShapeSuperKeyframe.cpp \
    SuperBlockGraph.cpp \
    TChainGraph.cpp \
    TBlockGraph.cpp

