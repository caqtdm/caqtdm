include (../caQtDM_Viewer/qtdefs.pri)
CONFIG += caQtDM_Lib
include(../caQtDM.pri)

QT += core gui

contains(QT_VER_MAJ, 4) {
   CONFIG   += qt thread uitools plugin
}

contains(QT_VER_MAJ, 5) {
    QT     += widgets printsupport uitools
    CONFIG += qt plugin thread
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

CONFIG   += warn_on

TARGET = caQtDM_Lib

TEMPLATE = lib
MOC_DIR = ./moc
VPATH += ./src
UI_DIR += ./

INCLUDEPATH += .
INCLUDEPATH += ./src
INCLUDEPATH += ./caQtDM_Plugins
INCLUDEPATH += ../caQtDM_QtControls/src
INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += $(EPICSINCLUDE)

SOURCES += caqtdm_lib.cpp \
    mutexKnobData.cpp \
    MessageWindow.cpp \
    vaPrintf.c \
    myMessageBox.cpp \
    limitsStripplotDialog.cpp \
    limitsCartesianplotDialog.cpp \
    limitsDialog.cpp \
    sliderDialog.cpp \
    splashscreen.cpp \
    loadPlugins.cpp
    
HEADERS += caqtdm_lib.h\
        caQtDM_Lib_global.h \
    mutexKnobDataWrapper.h \
    mutexKnobData.h \
    knobDefines.h \
    knobData.h \
    dbrString.h \
    alarmstrings.h \
    MessageWindow.h \
    messageWindowWrapper.h \
    vaPrintf.h \
    myMessageBox.h \
    limitsStripplotDialog.h \
    limitsDialog.h \
    limitsCartesianplotDialog.h \
    sliderDialog.h \
    splashscreen.h \
    epicsExternals.h \
    inlines.h \
    loadPlugins.h

!MOBILE {
    SOURCES += myQProcess.cpp  processWindow.cpp
    HEADERS += myQProcess.h  processWindow.h
}

HEADERS += JSONValue.h JSON.h
SOURCES += JSONValue.cpp JSON.cpp

#if we want some info from the australian lightsource, define it above
australian: {
  DEFINES +=_AUSTRALIAN
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/widgets/include
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/data/include
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/api/include
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/common
  LIBS += -L$(QTBASE)/designer -lQEPlugin
}

