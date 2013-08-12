include(qtdefs.pri)

QT       += core gui svg
CONFIG      += qt warn_on thread uitools

unix: {
  CONFIG += x11
}

MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src
DESTDIR = ../
UI_DIR += ../src

SOURCES +=\
    caQtDM.cpp \
    fileopenwindow.cpp \
    messagebox.cpp

HEADERS  +=  \
    messagebox.h \
    fileopenwindow.h \


FORMS += main.ui

INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += ../../caQtDM_QtControls/src
INCLUDEPATH += ../../caQtDM_Lib/src

DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"
DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"

RESOURCES += ./caQtDM.qrc
RC_FILE = ./caQtDM.rc
