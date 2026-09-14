TARGET_PRODUCT = "Library for Display Manager"
TARGET_FILENAME = "caQtDM_Lib.dll"

include (../caQtDM_Viewer/qtdefs.pri)
CONFIG += caQtDM_Lib
include(../caQtDM.pri)

QT += core gui network

win32 {
    LIBS += -ladvapi32
}

contains(QT_VER_MAJ, 4) {
   CONFIG += qt thread uitools plugin  qtestlib
}

contains(QT_VER_MAJ, 5) {
    QT += widgets  uitools opengl
    web {
        QT += websockets
        DEFINES += WEB
    }
    !ios:!android {
       message("caQtDM_Lib -- printsupport added")
       QT += printsupport
    }

    CONFIG += qt plugin thread cahmi
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

contains(QT_VER_MAJ, 6) {
    QT += widgets uitools opengl xml

    web {
        QT += websockets
        DEFINES += WEB
    }

    !ios:!android {
       message("caQtDM_Lib -- printsupport added")
       QT += printsupport
    }
    CONFIG += qt plugin thread cahmi
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050000
}

FORMS += ../caQtDM_Viewer/src/main.ui

CONFIG += warn_on

TARGET = caQtDM_Lib

TEMPLATE = lib
MOC_DIR = ./moc
VPATH += ./src
UI_DIR += ./

INCLUDEPATH += .
INCLUDEPATH += ./src
INCLUDEPATH += ../caQtDM_Plugins
INCLUDEPATH += ../caQtDM_QtControls/src
INCLUDEPATH += ../caQtDM_Parsers/adlParserSrc
INCLUDEPATH += ../caQtDM_Parsers/edlParserSrc
INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += $(EPICSINCLUDE)

android {
   QMAKE_CXXFLAGS += "-g"
   QMAKE_CFLAGS_RELEASE += "-g"
}

RC_FILE = ./src/caQtDM_Lib.rc

SOURCES += caqtdm_lib.cpp \
    mutexKnobData.cpp \
    MessageWindow.cpp \
    src/causerid.cpp \
    vaPrintf.c \
    myMessageBox.cpp \
    limitsStripplotDialog.cpp \
    limitsCartesianplotDialog.cpp \
    limitsDialog.cpp \
    sliderDialog.cpp \
    splashscreen.cpp \
    loadPlugins.cpp

cahmi{
SOURCES +=\
    src/hmisharedconfiglistmanager.cpp \
    src/hmisharedeventbus.cpp \
}

web {
SOURCES +=\
    websocketserver.cpp \
    vncwebchildprocess.cpp \
    webportpool.cpp \
    weblaunchermanager.cpp
}


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
    loadPlugins.h \
    caqtdm_lib_interface.h

cahmi{
HEADERS += \
    src/causerid.h \
    src/hmi_common_event_defs.h \
    src/hmisharedconfiglistmanager.h \
    src/hmisharedeventbus.h \
}

web {
HEADERS += \
    websocketserver.h \
    vncwebchildprocess.h \
    webportpool.h \
    weblaunchermanager.h
}




!MOBILE {
    SOURCES += myQProcess.cpp  processWindow.cpp
    HEADERS += myQProcess.h  processWindow.h
}

#if we want some info from the australian lightsource, define it above
australian: {
  DEFINES +=_AUSTRALIAN
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/widgets/include
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/data/include
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/api/include
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/common
  LIBS += -L$(QTBASE)/designer -lQEPlugin
}

FORMS +=
