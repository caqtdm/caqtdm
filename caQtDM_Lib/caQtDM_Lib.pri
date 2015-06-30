include(../caQtDM_Viewer/qtdefs.pri)

QT       += core gui

contains(QT_VER_MAJ, 4) {
   CONFIG   += qt thread uitools plugin
}

contains(QT_VER_MAJ, 5) {
    QT     += widgets printsupport uitools
    CONFIG += qt plugin thread
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

CONFIG   += warn_on

#CONFIG += epics4
#CONFIG += australian

unix {
 QMAKE_CXXFLAGS += "-g"
 QMAKE_CFLAGS_RELEASE += "-g"
}

PYTHONCALC: {
!ios {
   unix:!macx {
      DEFINES += PYTHON
      INCLUDEPATH += $(PYTHONINCLUDE)
      LIBS += -L$(PYTHONLIB) -Wl,-rpath,$(PYTHONLIB) -lpython$(PYTHONVERSION)
    }
    unix:macx {
       DEFINES += PYTHON
       INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/$(PYTHONVERSION)/include/python$(PYTHONVERSION)/
       LIBS += -L/System/Library/Frameworks/Python.framework/Versions/$(PYTHONVERSION)/lib/ -lpython$(PYTHONVERSION)
    }
  }
}

!ios {
   unix:!macx {
      LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca
      LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
      INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
   }
   macx: {
      LIBS += -L$(EPICSLIB)  -lca
      LIBS += -L$(CAQTDM_COLLECT)  -lqtcontrols
      INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
   }
}

ios {
      INCLUDEPATH += $(EPICSINCLUDE)
      INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
      SOURCES +=     fingerswipegesture.cpp
      HEADERS +=     fingerswipegesture.h
}

android {
      INCLUDEPATH += $(EPICSINCLUDE)
      INCLUDEPATH += $(EPICSINCLUDE)/os/android
      SOURCES +=     fingerswipegesture.cpp
      HEADERS +=     fingerswipegesture.h
}

SOURCES += caqtdm_lib.cpp \
    mutexKnobData.cpp \
    globalPtrs.cpp \
    epicsSubs.c \
    MessageWindow.cpp \
    vaPrintf.c \
    myMessageBox.cpp \
    limitsStripplotDialog.cpp \
    limitsCartesianplotDialog.cpp \
    limitsDialog.cpp \
    sliderDialog.cpp \
    processWindow.cpp \
    splashscreen.cpp \
    myQProcess.cpp


HEADERS += caqtdm_lib.h\
        caQtDM_Lib_global.h \
    mutexKnobDataWrapper.h \
    mutexKnobData.h \
    medmpio.h \
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
    processWindow.h \
    splashscreen.h \
    epicsExternals.h \
    myQProcess.h \
    inlines.h

HEADERS += \
    JSONValue.h \
    JSON.h

SOURCES += \
    JSONValue.cpp \
    JSON.cpp

#if we want to use epics4, define it above
epics4: {
SOURCES += ../src/epics4Subs.cpp pvAccessImpl.cpp
HEADERS += ../src/epics4Subs.h pvAccessImpl.h
EPICS4LIB = /usr/local/epics/base-4.2.0/lib/SL6-x86/
LIBS += -L$${EPICS4LIB} -Wl,-rpath,$${EPICS4LIB}  -lpvAccess -lpvData
INCLUDEPATH += /usr/local/epics/base-4.2.0/include
DEFINES += epics4
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

INCLUDEPATH += .
INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += $(EPICSINCLUDE)


