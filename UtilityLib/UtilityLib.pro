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

# Library name and destination
TARGET = UtilityLib
DESTDIR = $$PWD/$$CFG/lib

HEADERS += \
    UtilityGlobal.h \
    ProbabilityDistributions.h \
    Numeric.h \
    CustomDrawObjects.h \
    XmlWriter.h \
    RootFinder.h \
    TimeInterval.h

SOURCES += \
    Numeric.cpp \
    UtilityGlobal.cpp \
    XmlWriter.cpp \
    TimeInterval.cpp
