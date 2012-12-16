include (../caQtDM/qtdefs.pri)

unix {
  TEMPLATE = subdirs
  SUBDIRS = caQtDM_Lib1
  exists("/home/ACS/Control/Lib/libDEV.so") {
         SUBDIRS += caQtDM_Lib2
  }
}

win32 {
QT       += core gui

CONFIG      += qt warn_on thread uitools designer plugin

TARGET = caQtDM_Lib
TEMPLATE = lib

MOC_DIR = moc
OBJECTS_DIR = obj
DESTDIR = .
INCLUDEPATH += .
UI_DIR += ./

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



INCLUDEPATH += $${QWTHOME}/src
INCLUDEPATH += $${QTCONTROLS_INCLUDES}
INCLUDEPATH += $${EPICS_INCLUDES1}
INCLUDEPATH += $${EPICS_INCLUDES2}

LIBS += $${QWTHOME}/lib/libqwt.a
LIBS += $${QTCONTROLS_LIBS}/libqtcontrols.a
LIBS += $${EPICS_LIBS}/ca.lib
LIBS += $${EPICS_LIBS}/COM.lib

}
