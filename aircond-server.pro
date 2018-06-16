#-------------------------------------------------
#
# Project created by QtCreator 2018-05-12T15:06:32
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = aircond-server
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS

#win32:RC_ICONS += res/main.icon
win32:RC_FILE += res/main.rc

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    quiwidget.cpp \
    panelwidget.cpp \
    client.cpp \
    detaillist.cpp \
    dailyreport.cpp

HEADERS += \
    mainwindow.h \
    quiwidget.h \
    panelwidget.h \
    client.h \
    IconsFontAwesome5.h \
    detaillist.h \
    dailyreport.h

FORMS += \
    mainwindow.ui \
    client.ui \
    detaillist.ui \
    dailyreport.ui

RESOURCES += \
    res/qss.qrc \
    res/main.qrc

DISTFILES += \
    uncrustify.cfg
