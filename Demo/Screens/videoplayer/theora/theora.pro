#-------------------------------------------------
#
# Project created by QtCreator 2012-05-11T17:30:56
#
#-------------------------------------------------

QT       -= core gui

DESTDIR = $$PWD/../

TARGET = theora
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD ../ogg ../vorbis

SOURCES += \
    src/tokenize.c \
    src/state.c \
    src/rate.c \
    src/quant.c \
    src/mcenc.c \
    src/mathops.c \
    src/internal.c \
    src/info.c \
    src/idct.c \
    src/huffenc.c \
    src/huffdec.c \
    src/fragment.c \
    src/fdct.c \
    src/enquant.c \
    src/encode.c \
    src/encinfo.c \
    src/encfrag.c \
    src/dequant.c \
    src/decode.c \
    src/decinfo.c \
    src/bitpack.c \
    src/apiwrapper.c \
    src/analyze.c \
    src/encapiwrapper.c \
    src/decapiwrapper.c
HEADERS +=
