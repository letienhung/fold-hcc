#-------------------------------------------------
#
# Project created by QtCreator 2012-05-11T17:34:52
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = encoder_example
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../ogg ../vorbis ../theora


#unix:LIBS += -L/usr/local/lib -lmath
win32:LIBS += $$PWD/../ogg.lib $$PWD/../vorbis.lib $$PWD/../theora.lib


SOURCES += \
    main.c \
    getopt1.c \
    getopt.c

HEADERS += \
    getopt.h
