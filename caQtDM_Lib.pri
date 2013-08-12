QT       += core gui

CONFIG   += qt warn_on thread uitools designer plugin

#epics4
#CONFIG += epics4

QMAKE_CXXFLAGS += "-g"
QMAKE_CFLAGS_RELEASE += "-g"

TARGET = caQtDM_Lib
TEMPLATE = lib

MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src
DESTDIR = ../
UI_DIR += ../

SOURCES += caqtdm_lib.cpp \
    mutexKnobData.cpp \
    globalPtrs.cpp \
    epicsSubs.c \
    dmsearchfile.cpp \
    MessageWindow.cpp \
    vaPrintf.c \
    myMessageBox.cpp \
    limitsStripplotDialog.cpp \
    limitsCartesianplotDialog.cpp \
    processWindow.cpp

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
    myMessageBox.h \
    limitsStripplotDialog.h \
    limitsCartesianplotDialog.h \
    processWindow.h

HEADERS += \
    JSONValue.h \
    JSON.h

SOURCES += \
    JSONValue.cpp \
    JSON.cpp

#if we want to use epics4, define it in caQtDM_Lib.pri
epics4: {
SOURCES += ../src/epics4Subs.cpp pvAccessImpl.cpp
HEADERS += ../src/epics4Subs.h pvAccessImpl.h
EPICS4LIB = /usr/local/epics/base-4.2.0/lib/SL6-x86/
LIBS += -L$${EPICS4LIB} -Wl,-rpath,$${EPICS4LIB}  -lpvAccess -lpvData
INCLUDEPATH += /usr/local/epics/base-4.2.0/include
DEFINES += epics4
}

INCLUDEPATH += .

LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca
LIBS += -L$(QTBASE) -Wl,-rpath,$(QTCONTROLS_INSTALLED) -lqtcontrols

INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += $(EPICSINCLUDE)
INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
INCLUDEPATH += ../../caQtDM_QtControls/src


