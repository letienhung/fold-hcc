load($$[STARLAB])
load($$[SURFACEMESH])

StarlabTemplate(plugin)


# Build flag
CONFIG(debug, debug|release) {
    CFG = debug
} else {
    CFG = release
}

# Foldabilizer library
LIBS += -L$$PWD/../FoldabilizerLib/$$CFG/lib -lFoldabilizerLib
INCLUDEPATH += ../FoldabilizerLib

HEADERS += \
    FuiWidget.h \
    FoldabilizerUi.h

SOURCES += \
    FuiWidget.cpp \
    FoldabilizerUi.cpp

RESOURCES += \
    FoldabilizerUi.qrc

FORMS += \
    FuiWidget.ui
