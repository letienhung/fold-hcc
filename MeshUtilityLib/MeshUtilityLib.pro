load($$[STARLAB])
load($$[SURFACEMESH])


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
TARGET = MeshUtilityLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    SegMeshLoader.h \
    QuickMeshDraw.h \
    MeshMerger.h \
    MeshHelper.h \
    MeshBoolean.h

SOURCES += \
    SegMeshLoader.cpp \
    MeshMerger.cpp \
    MeshHelper.cpp \
    MeshBoolean.cpp
