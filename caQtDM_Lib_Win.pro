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


QWT_HOME = C:/Qwt-6.0.1/Qwt
INCLUDEPATH += $${QWT_HOME}/src
INCLUDEPATH += C:/work/QtControls/qtcontrols/src
INCLUDEPATH += C:\epics\base-3.14.12\include
INCLUDEPATH += C:\epics\base-3.14.12\include\os\WIN32
LIBS += $${QWT_HOME}/lib/libqwt.a
LIBS += C:/work/QtControls/qtcontrols/release/libqtcontrols.a
LIBS += C:\epics\base-3.14.12\lib\win32-x86-mingw/ca.lib
LIBS += C:\epics\base-3.14.12\lib\win32-x86-mingw/COM.lib


