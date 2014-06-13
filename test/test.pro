QT       += testlib gui multimedia

QT       -= gui

TARGET = test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += test.cpp \
    main.cpp \
    ../client/frame.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(../common.pri)

HEADERS += \
    test.h

