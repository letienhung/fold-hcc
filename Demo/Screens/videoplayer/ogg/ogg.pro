#-------------------------------------------------
#
# Project created by QtCreator 2012-05-11T17:23:29
#
#-------------------------------------------------

QT       -= core gui

DESTDIR = $$PWD/../

TARGET = ogg
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += .

SOURCES += \
    ogg/src/framing.c \
    ogg/src/bitwise.c

HEADERS += \
    ogg/os_types.h \
    ogg/ogg.h

