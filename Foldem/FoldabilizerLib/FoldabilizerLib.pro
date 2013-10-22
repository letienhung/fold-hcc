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
    Graph.h

SOURCES += \
    Graph.cpp
