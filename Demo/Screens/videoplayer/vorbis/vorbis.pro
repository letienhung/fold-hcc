#-------------------------------------------------
#
# Project created by QtCreator 2012-05-11T17:27:27
#
#-------------------------------------------------

QT       -= core gui

DESTDIR = $$PWD/../

TARGET = vorbis
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD ../ogg

SOURCES += \
    vorbis/src/window.c \
    vorbis/src/vorbisfile.c \
    vorbis/src/vorbisenc.c \
    vorbis/src/synthesis.c \
    vorbis/src/smallft.c \
    vorbis/src/sharedbook.c \
    vorbis/src/res0.c \
    vorbis/src/registry.c \
    vorbis/src/psy.c \
    vorbis/src/mdct.c \
    vorbis/src/mapping0.c \
    vorbis/src/lsp.c \
    vorbis/src/lpc.c \
    vorbis/src/lookup.c \
    vorbis/src/info.c \
    vorbis/src/floor1.c \
    vorbis/src/floor0.c \
    vorbis/src/envelope.c \
    vorbis/src/codebook.c \
    vorbis/src/block.c \
    vorbis/src/bitrate.c \
    vorbis/src/analysis.c
HEADERS += \
    vorbis/vorbisfile.h \
    vorbis/vorbisenc.h \
    vorbis/codec.h
