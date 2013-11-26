load($$[STARLAB])
load($$[SURFACEMESH])
include($$[CHOLMOD])
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
    DeformHandle.h\
	Deformer.h\
    FuiWidget.h \
	TransformationPanel.h\
    FoldabilizerUi.h

SOURCES += \
    FuiWidget.cpp \
	Deformer.cpp\
	TransformationPanel.cpp\
    FoldabilizerUi.cpp

RESOURCES += \
    FoldabilizerUi.qrc

FORMS += \
    FuiWidget.ui\
	RotationWidget.ui\
	TranslationWidget.ui\
	ScaleWidget.ui
