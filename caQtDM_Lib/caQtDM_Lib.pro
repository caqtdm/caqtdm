include (../caQtDM_Viewer/qtdefs.pri)

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

unix {
 QMAKE_CXXFLAGS += "-g"
 QMAKE_CFLAGS_RELEASE += "-g"
}


unix {
TARGET = caQtDM_Lib
OBJECTS_DIR = ./obj
TEMPLATE = lib
MOC_DIR = ./moc
VPATH += ./src
INCLUDEPATH += ./src
UI_DIR += ./
}

INCLUDEPATH += ../caQtDM_QtControls/src
INCLUDEPATH += .
INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += $(EPICSINCLUDE)

DESTDIR = $(CAQTDM_COLLECT)

PYTHONCALC: {
  warning("for image and visibility calculation, python will be build in")
!ios {
!android {
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
}

ios | android {
   CONFIG += staticlib
}

!ios {
   unix:!macx {
      LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca
      LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
      INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
      INCLUDEPATH += ./caQtDM_Plugins
   }
   macx: {
      INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
      INCLUDEPATH += ./caQtDM_Plugins
      QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
      LIBS += -F$(QWTLIB) -framework qwt
      LIBS += ${EPICSLIB}/libca.dylib
      LIBS += ${EPICSLIB}/libCom.dylib
   }
}

ios {
      INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
      SOURCES +=     fingerswipegesture.cpp
      HEADERS +=     fingerswipegesture.h
}

android {
      INCLUDEPATH += $(EPICSINCLUDE)/os/android
      SOURCES +=     fingerswipegesture.cpp
      HEADERS +=     fingerswipegesture.h
}

SOURCES += caqtdm_lib.cpp \
    mutexKnobData.cpp \
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

#if we want some info from the australian lightsource, define it above
australian: {
  DEFINES +=_AUSTRALIAN
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/widgets/include
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/data/include
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/api/include
  INCLUDEPATH += ../../../epicsQt/2.8.1/framework/common
  LIBS += -L$(QTBASE)/designer -lQEPlugin
}

win32 {
  win32-msvc* {
        DEFINES +=_CRT_SECURE_NO_WARNINGS
        DEFINES += CAQTDM_LIB_LIBRARY
        TEMPLATE = lib
        
        DebugBuild {
                EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
                DESTDIR = $(CAQTDM_COLLECT)/debug
                OBJECTS_DIR = debug/obj
                LIBS += $$(QWTHOME)/lib/qwtd.lib
                LIBS += $${EPICS_LIBS}/ca.lib
                LIBS += $${EPICS_LIBS}/COM.lib
                LIBS += $(CAQTDM_COLLECT)/debug/qtcontrols.lib
        }
        ReleaseBuild {
                QMAKE_CXXFLAGS += /Z7
                QMAKE_CFLAGS   += /Z7
                QMAKE_LFLAGS   += /DEBUG /OPT:REF /OPT:ICF
                EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
                DESTDIR = $(CAQTDM_COLLECT)
                OBJECTS_DIR = release/obj
                LIBS += $$(QWTHOME)/lib/qwt.lib
                LIBS += $${EPICS_LIBS}/ca.lib
                LIBS += $${EPICS_LIBS}/COM.lib
                LIBS += $(CAQTDM_COLLECT)/qtcontrols.lib
                
        }
   }
   win32-g++ {
       EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
	LIBS += $$(QWTLIB)/libqwt.a
	LIBS += $$(QTCONTROLS_LIBS)/release//libqtcontrols.a
	LIBS += $${EPICS_LIBS}/ca.lib
	LIBS += $${EPICS_LIBS}/COM.lib
	QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\caQtDM_Lib.dll ..\caQtDM_Binaries
   }

   INCLUDEPATH += $$(EPICS_BASE)/include
   INCLUDEPATH += $$(EPICS_BASE)/include/os/win32
}

