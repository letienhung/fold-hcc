include($$[STARLAB])
include($$[SURFACEMESH])
StarlabTemplate(appbundle)

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DemoApp
TEMPLATE = app


SOURCES     += main.cpp MainWindow.cpp \
    Viewer.cpp
HEADERS     += MainWindow.h \
    Viewer.h

FORMS       += MainWindow.ui \
    Viewer.ui


# Build flag
CONFIG(debug, debug|release) {
    CFG = debug
} else {
    CFG = release
}

### Libraries:
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

# Icons and images
RESOURCES += resources.qrc
win32:RC_FILE = resources.rc
