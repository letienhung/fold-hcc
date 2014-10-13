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
    FoldManager.h \
    FdUtility.h \
    Hinge.h \
    BundleNode.h \
    Decomposer.h \
    Scaffold.h \
    ChainScaff.h \
    DecScaff.h \
    HChainScaff.h \
    HUnitScaff.h \
    ScaffLink.h \
    ScaffManager.h \
    ScaffNode.h \
    SuperUnitScaff.h \
    TChainScaff.h \
    TUnitScaff.h \
    UnitScaff.h \
    ZUnitScaff.h \
    SuperShapeKf.h \
    FoldOptGraph.h \
    VisualDebugger.h

SOURCES += \
    PatchNode.cpp \
    RodNode.cpp \
    Scaffold.cpp \
    FoldManager.cpp \
    FdUtility.cpp \
    Hinge.cpp \
    BundleNode.cpp \
    Decomposer.cpp \
    ChainScaff.cpp \
    DecScaff.cpp \
    HChainScaff.cpp \
    HUnitScaff.cpp \
    ScaffLink.cpp \
    ScaffManager.cpp \
    ScaffNode.cpp \
    SuperUnitScaff.cpp \
    TChainScaff.cpp \
    TUnitScaff.cpp \
    UnitScaff.cpp \
    ZUnitScaff.cpp \
    SuperShapeKf.cpp \
    FoldOptGraph.cpp \
    VisualDebugger.cpp

