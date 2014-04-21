TEMPLATE = lib
CONFIG += staticlib


# Build flag
CONFIG(debug, debug|release) {
    CFG = debug
} else {
    CFG = release
}

# Library name and destination
TARGET = CliquerLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    cliquer.h \
    misc.h \
    reorder.h \
    set.h \
    cliquerconf.h \
    cliquer_graph.h

SOURCES += \
    cliquer.c \
    reorder.c \
    cliquer_graph.c
