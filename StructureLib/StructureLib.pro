TEMPLATE = lib
CONFIG += staticlib


# Build flag
CONFIG(debug, debug|release) {
    CFG = debug
} else {
    CFG = release
}

# Library name and destination
TARGET = StructureLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    Node.h \
    Link.h \
    Graph.h \
    PropertyMap.h

SOURCES += \
    Link.cpp \
    Graph.cpp \
    Node.cpp
