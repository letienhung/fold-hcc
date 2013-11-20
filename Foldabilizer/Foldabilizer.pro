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

# Foldabilizer library
LIBS += -L$$PWD/../FoldabilizerLib/$$CFG/lib -lFoldabilizerLib
INCLUDEPATH += ../FoldabilizerLib

HEADERS += \
    Foldabilizer.h \
    foldem_widget.h

SOURCES += \
    Foldabilizer.cpp \
    foldem_widget.cpp

RESOURCES += \
    foldabilizer.qrc

FORMS += \
    foldem_widget.ui
