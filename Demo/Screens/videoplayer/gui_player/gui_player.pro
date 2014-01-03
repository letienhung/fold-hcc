#-------------------------------------------------
#
# Project created by QtCreator 2012-05-11T21:13:57
#
#-------------------------------------------------

QT       += core gui

TARGET = example
TEMPLATE = app

DEFINES += THEORAVIDEO_STATIC _DEBUG_VIDEO

INCLUDEPATH += $$PWD ../ogg ../vorbis ../theora ../theoraplayer/include/theoraplayer

win32:LIBS += $$PWD/../theoraplayer.lib $$PWD/../ogg.lib $$PWD/../vorbis.lib $$PWD/../theora.lib

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    VideoWidget.h \
    global.h \
    VideoToolbar.h

FORMS    += mainwindow.ui \
    VideoToolbar.ui
