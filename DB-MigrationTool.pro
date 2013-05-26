#-------------------------------------------------
#
# Project created by QtCreator 2013-05-02T14:20:44
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DB-MigrationTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    enhancedsqltablemodel.cpp \
    changedbs.cpp

HEADERS  += mainwindow.h \
    enhancedsqltablemodel.h \
    changedbs.h

FORMS    += mainwindow.ui \
    changedbs.ui
