QT       += core gui svg
CONFIG      += qt warn_on thread uitools

MOC_DIR = ../moc
VPATH += ../
OBJECTS_DIR = ../obj
DESTDIR = ../
INCLUDEPATH += ../
UI_DIR += ../

QTCONTROLS = $(QTBASE)/binQt

SOURCES +=\
    caQtDM.cpp \
    fileopenwindow.cpp \
    messagebox.cpp

HEADERS  +=  \
    messagebox.h \
    fileopenwindow.h \


FORMS += main.ui

INCLUDEPATH += $(QWTHOME)/src
INCLUDEPATH += $(HOME)/workarea/ACS/mezger/QtControls/qtcontrols/src
INCLUDEPATH += $(HOME)/workarea/ACS/mezger/caQtDM_Lib

DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"
DEFINES += BUILDVERSION=\\\"V2.4\\\"
DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"


RESOURCES +=
