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
    ScaffoldManager.h \
    ScaffoldLink.h \
    ScaffoldNode.h \
    FoldManager.h \
    FdUtility.h \
    UnitScaffold.h \
    ChainScaffold.h \
    Hinge.h \
    BundleNode.h \
    FoldOptionGraph.h \
    ShapeSuperKeyframe.h \
    SuperUnitScaffold.h \
    TChainScaffold.h \
    TUnitScaffold.h \
    HUnitScaffold.h \
    HChainScaffold.h \
    Decomposer.h \
    Scaffold.h \
    DecScaffold.h

SOURCES += \
    PatchNode.cpp \
    RodNode.cpp \
    ScaffoldManager.cpp \
    Scaffold.cpp \
    ScaffoldLink.cpp \
    ScaffoldNode.cpp \
    FoldManager.cpp \
    FdUtility.cpp \
    UnitScaffold.cpp \
    ChainScaffold.cpp \
    Hinge.cpp \
    BundleNode.cpp \
    FoldOptionGraph.cpp \
    ShapeSuperKeyframe.cpp \
    SuperUnitScaffold.cpp \
    TChainScaffold.cpp \
    TUnitScaffold.cpp \
    HUnitScaffold.cpp \
    HChainScaffold.cpp \
    Decomposer.cpp \
    DecScaffold.cpp

