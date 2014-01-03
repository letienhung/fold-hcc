#-------------------------------------------------
#
# Project created by QtCreator 2012-05-11T21:06:30
#
#-------------------------------------------------

QT       -= core gui

DESTDIR = $$PWD/../

TARGET = theoraplayer
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += ../ogg ../vorbis ../theora include/theoraplayer
win32:LIBS += $$PWD/../ogg.lib $$PWD/../vorbis.lib $$PWD/../theora.lib

DEFINES += THEORAVIDEO_STATIC

SOURCES += \
    src/TheoraWorkerThread.cpp \
    src/TheoraVideoManager.cpp \
    src/TheoraVideoFrame.cpp \
    src/TheoraVideoClip.cpp \
    src/TheoraUtil.cpp \
    src/TheoraTimer.cpp \
    src/TheoraFrameQueue.cpp \
    src/TheoraException.cpp \
    src/TheoraDataSource.cpp \
    src/TheoraAudioInterface.cpp \
    src/TheoraAsync.cpp

HEADERS += \
    src/Endianess.h \
    include/theoraplayer/TheoraWorkerThread.h \
    include/theoraplayer/TheoraVideoManager.h \
    include/theoraplayer/TheoraVideoFrame.h \
    include/theoraplayer/TheoraVideoClip.h \
    include/theoraplayer/TheoraUtil.h \
    include/theoraplayer/TheoraTimer.h \
    include/theoraplayer/TheoraPlayer.h \
    include/theoraplayer/TheoraFrameQueue.h \
    include/theoraplayer/TheoraExport.h \
    include/theoraplayer/TheoraException.h \
    include/theoraplayer/TheoraDataSource.h \
    include/theoraplayer/TheoraAudioInterface.h \
    include/theoraplayer/TheoraAsync.h
