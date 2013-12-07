load($$[STARLAB])
load($$[SURFACEMESH])

StarlabTemplate(plugin)


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

# Foldabilizer library
LIBS += -L$$PWD/../FoldabilizerLib/$$CFG/lib -lFoldabilizerLib
INCLUDEPATH += ../FoldabilizerLib

HEADERS += \
    Foldabilizer.h \
    FoldabilizerWidget.h

SOURCES += \
    Foldabilizer.cpp \
    FoldabilizerWidget.cpp

RESOURCES += \
    Foldabilizer.qrc

FORMS += \
    FoldabilizerWidget.ui
