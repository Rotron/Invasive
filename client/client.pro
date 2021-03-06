QT       += core gui multimedia opengl
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
    frame.cpp \
    abstractframedetector.cpp \
    framedetector.cpp \
    frameprinter.cpp \
    configdialog.cpp \
    socketserver.cpp \
    waterfallview.cpp \
    ../vendor/fft/fft8g.c \
    framelogger.cpp \
    demodulatorset.cpp \
    demodulator.cpp \
    wavplayer.cpp

HEADERS  += \
    mainwindow.h \
    framelistwidget.h \
    modem.h \
    firfilter.h \
    frameaudio.h \
    stdafx.h \
    forward.h \
    abstractdemodulator.h \
    frame.h \
    abstractframedetector.h \
    framedetector.h \
    ringbuffer.h \
    frameprinter.h \
    configdialog.h \
    socketserver.h \
    waterfallview.h \
    framelogger.h \
    demodulatorset.h \
    window.h \
    demodulator.h \
    wavplayer.h

win32:LIBS += -lglew32
linux:LIBS += -lGLEW

PRECOMPILED_HEADER = stdafx.h
precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}

OTHER_FILES += Info.plist
ICON = ../icon/invasive.icns
RC_FILE = ../icon/icon.rc

include(../common.pri)
