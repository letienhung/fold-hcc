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

# Mesh Utility Library
LIBS += -L$$PWD/../MeshUtilityLib/$$CFG/lib -lMeshUtilityLib
INCLUDEPATH += ../MeshUtilityLib

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

win32:LIBS += -lopengl32 \
    -lglu32\
    -l./Screens/videoplayer/ogg \
    -l./Screens/videoplayer/vorbis \
    -l./Screens/videoplayer/theora \
    -l./Screens/videoplayer/theoraplayer
    #-L"./Screens/project/GUI/Viewer/libQGLViewer/QGLViewer/lib" \
    #-lQGLViewer2 \


HEADERS += ./Screens/MyDesigner.h \
    ./Screens/UiUtility/BBox.h \
    ./Screens/UiUtility/SimpleDraw.h\
	./Screens/UiUtility/GL/VBO/VBO.h\
    ./Screens/UiUtility/GL/Glee.h\
    ./resource.h \
    ./MainWindow.h \
    ./Screens/videoplayer/gui_player/VideoToolbar.h \
    ./Screens/videoplayer/gui_player/VideoWidget.h\
	./Screens/UiUtility/QuickMeshViewer.h\
	./Screens/MyAnimator.h

SOURCES += ./Screens/MyDesigner.cpp \
    ./Screens/UiUtility/BBox.cpp \
    ./Screens/UiUtility/SimpleDraw.cpp\
    ./Screens/UiUtility/GL/VBO/VBO.cpp \
    ./Screens/UiUtility/GL/Glee.c\
    ./main.cpp \
    ./MainWindow.cpp\
    ./Screens/UiUtility/QuickMeshViewer.cpp\
    ./Screens/MyAnimator.cpp
	
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/release
OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles

FORMS += ./Screens/DesignWidget.ui \
    ./Screens/TutorialWidget.ui\
    ./MainWindow.ui\
	./Screens/EvaluateWidget.ui\
	./Screens/videoplayer/gui_player/VideoToolbar.ui
RESOURCES += Resources/Resource.qrc
