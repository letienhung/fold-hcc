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

# Library name and destination
TARGET = FoldabilizerLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    Graph.h \
    Node.h \
    Edge.h \
    FoldabilizerLibGlobal.h \
    Box.h

SOURCES += \
    Graph.cpp \
    Node.cpp \
    Edge.cpp \
    Box.cpp
