QT       += core gui multimedia
CONFIG   += precompile_header

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Invasive
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    framelistwidget.cpp \
    modem.cpp \
    firfilter.cpp \
    frameaudio.cpp \
    abstractdemodulator.cpp \
    defaultdemodulator.cpp \
    frame.cpp \
    abstractframedetector.cpp \
    framedetector.cpp \
    frameprinter.cpp \
    configdialog.cpp \
    socketserver.cpp

HEADERS  += \
    mainwindow.h \
    framelistwidget.h \
    modem.h \
    firfilter.h \
    frameaudio.h \
    stdafx.h \
    forward.h \
    abstractdemodulator.h \
    defaultdemodulator.h \
    frame.h \
    abstractframedetector.h \
    framedetector.h \
    ringbuffer.h \
    frameprinter.h \
    configdialog.h \
    socketserver.h

PRECOMPILED_HEADER = stdafx.h
precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}

OTHER_FILES += Info.plist
ICON = ../icon/invasive.icns

include(../common.pri)
