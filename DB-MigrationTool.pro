#-------------------------------------------------
#
# Project created by QtCreator 2013-05-02T14:20:44
#
#-------------------------------------------------

QT       += core gui sql

CONFIG  += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DB-MigrationTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    enhancedsqltablemodel.cpp \
    changedbs.cpp \
    dbtableview.cpp \
    step2.cpp \
    tabview.cpp

HEADERS  += mainwindow.h \
    enhancedsqltablemodel.h \
    changedbs.h \
    dbtableview.h \
    step2.h \
    tabview.h

FORMS    += mainwindow.ui \
    changedbs.ui \
    step2.ui

RESOURCES += \
    rc.qrc
