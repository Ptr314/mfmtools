#-------------------------------------------------
#
# Project created by QtCreator 2017-02-23T02:06:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION,5): QT += core5compat

TARGET = MFMTools
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CFLAGS += -std=c99
QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-parameter

SOURCES += main.cpp\
        mainwindow.cpp \
    converter.cpp \
    libs/crc16.c \
    libs/loader.cpp \
    libs/loader_raw.cpp \
    libs/writer.cpp \
    libs/writer_mfm.cpp \
    diskparamsdlg.cpp \
    libs/loader_td0.cpp \
    loggerdlg.cpp \
    libs/writer_raw.cpp \
    libs/lzss.c \
    libs/disk_formats.cpp \
    libs/hcombo.cpp

HEADERS  += mainwindow.h \
    libs/mfm_formats.h \
    converter.h \
    libs/crc16.h \
    libs/disk_formats.h \
    libs/loader.h \
    libs/loader_raw.h \
    libs/writer.h \
    libs/writer_mfm.h \
    diskparamsdlg.h \
    libs/loader_td0.h \
    loggerdlg.h \
    libs/writer_raw.h \
    libs/lzss.h \
    libs/config.h \
    libs/hcombo.h

FORMS    += mainwindow.ui \
    diskparamsdlg.ui \
    loggerdlg.ui \
    aboutdlg.ui

RESOURCES += \
    resources.qrc

TRANSLATIONS =  deploy/languages/app_ru.ts\
                deploy/languages/app_en.ts
