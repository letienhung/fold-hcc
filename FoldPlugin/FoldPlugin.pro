include($$[STARLAB])
include($$[SURFACEMESH])

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

# Mesh Utility library
LIBS += -L$$PWD/../MeshUtilityLib/$$CFG/lib -lMeshUtilityLib
INCLUDEPATH += ../MeshUtilityLib

# Cliquer library
LIBS += -L$$PWD/../CliquerLib/$$CFG/lib -lCliquerLib
INCLUDEPATH += ../CliquerLib

# Foldabilizer library
LIBS += -L$$PWD/../FoldLib/$$CFG/lib -lFoldLib
INCLUDEPATH += ../FoldLib

HEADERS += \
    FdPlugin.h \
    FdWidget.h

SOURCES += \
    FdWidget.cpp \
    FdPlugin.cpp

RESOURCES += \
    FoldPlugin.qrc

FORMS += \
    FdWidget.ui
