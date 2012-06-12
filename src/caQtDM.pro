#-------------------------------------------------
#
# Project created by QtCreator 2011-11-22T17:25:27
#
#-------------------------------------------------

QT       += core gui svg
CONFIG      += qt warn_on thread uitools

TARGET = caQtDM
TEMPLATE = app

SOURCES +=\
    caQtDM.cpp \
    fileopenwindow.cpp \
    messagebox.cpp


HEADERS  +=  \
    messagebox.h \
    fileopenwindow.h


FORMS    += \
    main.ui

MOC_DIR = moc
OBJECTS_DIR = obj
DESTDIR = .

INCLUDEPATH += .
win32 {
     QWT_HOME = C:/Qwt-6.0.1/Qwt
     INCLUDEPATH += $${QWT_HOME}/src
     INCLUDEPATH += C:/work/QtControls/qtcontrols/src
     INCLUDEPATH += C:/work/caQtDM_Lib
     INCLUDEPATH += C:\epics\base-3.14.12\include
     INCLUDEPATH += C:\epics\base-3.14.12\include\os\WIN32
     LIBS += $${QWT_HOME}/lib/libqwt.a
     LIBS += C:/work/QtControls/qtcontrols/release/libqtcontrols4.a
     LIBS += C:\epics\base-3.14.12\lib\win32-x86-mingw/ca.lib
     LIBS += C:\epics\base-3.14.12\lib\win32-x86-mingw/COM.lib
     LIBS += C:/work/caQtDM_Lib/release/libcaQtDm_Lib1.a
}

unix {
    INCLUDEPATH += /usr/local/Trolltech/qwt-6.0.0/src
    INCLUDEPATH += $(EPICSINCLUDE)
    INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
    INCLUDEPATH += $(HOME)/workarea/ACS/mezger/QtControls/qtcontrols/src
    INCLUDEPATH += $(HOME)/workarea/ACS/mezger/caQtDM_Lib
    LIBS += -L/usr/local/Trolltech/qwt-6.0.0/lib -lqwt
    LIBS += -L$(EPICSLIB) -lca
    LIBS += -L$(HOME)/workarea/ACS/mezger/QtControls/qtcontrols -lqtcontrols
    LIBS += -L$(HOME)/workarea/ACS/mezger/caQtDM_Lib -lcaQtDM_Lib
}

unix {
DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"
}
win32 {
DEFINES += BUILDTIME=\\\"\\\"
DEFINES += BUILDDATE=\\\"\\\"
}
DEFINES += BUILDVERSION=\\\"V1.4\\\"

UI_DIR += ./

RESOURCES +=
