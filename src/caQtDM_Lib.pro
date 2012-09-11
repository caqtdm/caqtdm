#-------------------------------------------------
#
# Project created by QtCreator 2012-05-19T20:07:24
#
#-------------------------------------------------

QT       += core gui

CONFIG      += qt warn_on thread uitools

TARGET = caQtDM_Lib
TEMPLATE = lib

MOC_DIR = moc
OBJECTS_DIR = obj
DESTDIR = .

VERSION = 1.0.0

DEFINES += CAQTDM_LIB_LIBRARY

SOURCES += caqtdm_lib.cpp \
    mutexKnobData.cpp \
    globalPtrs.cpp \
    epicsSubs.c \
    dmsearchfile.cpp \
    MessageWindow.cpp \
    vaPrintf.c \
    myMessageBox.cpp

HEADERS += caqtdm_lib.h\
        caQtDM_Lib_global.h \
    mutexKnobDataWrapper.h \
    mutexKnobData.h \
    medmpio.h \
    knobDefines.h \
    knobData.h \
    dbrString.h \
    alarmdefs.h \
    dmsearchfile.h \
    MessageWindow.h \
    messageWindowWrapper.h \
    vaPrintf.h \
    myMessageBox.h

INCLUDEPATH += .
win32 {
     QWT_HOME = C:/Qwt-6.0.1/Qwt
     INCLUDEPATH += $${QWT_HOME}/src
     INCLUDEPATH += C:/work/QtControls/qtcontrols/src
     INCLUDEPATH += C:\epics\base-3.14.12\include
     INCLUDEPATH += C:\epics\base-3.14.12\include\os\WIN32
     LIBS += $${QWT_HOME}/lib/libqwt.a
     LIBS += C:/work/QtControls/qtcontrols/release/libqtcontrols4.a
     LIBS += C:\epics\base-3.14.12\lib\win32-x86-mingw/ca.lib
     LIBS += C:\epics\base-3.14.12\lib\win32-x86-mingw/COM.lib

}

unix {
    LIBS += -L/usr/local/Trolltech/qwt-6.0.0/lib -lqwt
    LIBS += -L$(EPICSLIB) -lca
    LIBS += -L$(HOME)/workarea/ACS/mezger/QtControls/qtcontrols -lqtcontrols
    INCLUDEPATH += /usr/local/Trolltech/qwt-6.0.0/src
    INCLUDEPATH += $(EPICSINCLUDE)
    INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
    INCLUDEPATH += $(HOME)/workarea/ACS/mezger/QtControls/qtcontrols/src
    DEPENDPATH +=/usr/local/Trolltech/qwt-6.0.0/src
}

#will build besides epics also acs if enabled
CONFIG += acs
acs: {
SOURCES += acsSubs.c \
           medmblock.c

HEADERS += acsSubs.h
LIBS += -L$(ACS_BUILD_LIBRARY) -lDEV -lCDB -lInclude -lProfAcc
INCLUDEPATH += $(ACS_BUILD_INCLUDE)
DEFINES += ACS
}

DEPENDPATH +=/usr/local/Trolltech/qwt-6.0.0/src

UI_DIR += ./

RESOURCES +=
