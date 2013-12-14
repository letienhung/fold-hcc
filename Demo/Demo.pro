include($$[STARLAB])
include($$[SURFACEMESH])
StarlabTemplate(appbundle)

# Build flag for the static libraries
CONFIG(debug, debug|release) {
    CFG = debug
} else {
    CFG = release
}

# Foldabilizer Library
LIBS += -L$$PWD/../FoldLib/$$CFG/lib -lFoldLib
INCLUDEPATH += ../FoldLib

# Geometry Library
LIBS += -L$$PWD/../GeometryLib/$$CFG/lib -lGeometryLib
INCLUDEPATH += ../GeometryLib

# Structure Library
LIBS += -L$$PWD/../StructureLib/$$CFG/lib -lStructureLib
INCLUDEPATH += ../StructureLib

# Utility Library
LIBS += -L$$PWD/../UtilityLib/$$CFG/lib -lUtilityLib
INCLUDEPATH += ../UtilityLib

# TEMPLATE = app
TARGET = Demo
DESTDIR = ../
QT += core gui multimedia network xml xmlpatterns webkit opengl
# CONFIG += release
DEFINES += QT_LARGEFILE_SUPPORT QT_MULTIMEDIA_LIB QT_XML_LIB QT_OPENGL_LIB QT_NETWORK_LIB QT_WEBKIT_LIB QT_XMLPATTERNS_LIB THEORAVIDEO_STATIC
INCLUDEPATH += ./GeneratedFiles \
    ./GeneratedFiles/Release \
    ./Screens/videoplayer \
    ./Screens/videoplayer/theoraplayer/include/theoraplayer

win32:LIBS += -L"./Screens/project/GUI/Viewer/libQGLViewer/QGLViewer/lib" \
    -lopengl32 \
    -lglu32 \
    -lQGLViewer2 \
    -l./Screens/videoplayer/ogg \
    -l./Screens/videoplayer/vorbis \
    -l./Screens/videoplayer/theora \
    -l./Screens/videoplayer/theoraplayer

HEADERS += ./Screens/MyDesigner.h \
    ./Screens/UiUtility/drawCube.h \
    ./Screens/UiUtility/drawPlane.h \
    ./Screens/UiUtility/drawRoundRect.h \
    ./Screens/UiUtility/sphereDraw.h \
    ./Screens/UiUtility/QManualDeformer.h \
    ./Screens/UiUtility/BBox.h \
    ./resource.h \
    ./MainWindow.h \
    ./Screens/videoplayer/gui_player/VideoToolbar.h \
    ./Screens/videoplayer/gui_player/VideoWidget.h

SOURCES += ./Screens/MyDesigner.cpp \
    ./Screens/UiUtility/QManualDeformer.cpp\
    ./Screens/UiUtility/BBox.cpp \
    ./Screens/UiUtility/Triangle.cpp \
    ./Screens/UiUtility/SimpleDraw.cpp\
    ./main.cpp \
    ./MainWindow.cpp
	
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/release
OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles

FORMS += ./Screens/DesignWidget.ui \
    ./Screens/TutorialWidget.ui\
    ./MainWindow.ui
RESOURCES += Resources/Resource.qrc
