include(qtdefs.pri)

QT       += core gui svg
CONFIG      += qt warn_on thread uitools

MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src
DESTDIR = ../
UI_DIR += ../src

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
INCLUDEPATH += $(HOME)/workarea/ACS/mezger/caQtDM_Lib/src

DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"
DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"

RESOURCES += ../caQtDM.qrc

