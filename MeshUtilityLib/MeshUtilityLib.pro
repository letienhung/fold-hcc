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

# Library name and destination
TARGET = MeshUtilityLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    SegMeshLoader.h \
    QuickMeshDraw.h \
    MeshMerger.h \
    MeshSplitter.h

SOURCES += \
    SegMeshLoader.cpp \
    MeshMerger.cpp \
    MeshSplitter.cpp
