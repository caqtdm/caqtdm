include (../caQtDM_Lib.pri)

TARGET = caQtDM_Lib
OBJECTS_DIR = ../obj1
TEMPLATE = lib
MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src

UI_DIR += ../
INCLUDEPATH += ../../caQtDM_QtControls/src
DESTDIR = $(CAQTDM_COLLECT)
!ios {

   macx: {

      INCLUDEPATH += $$(EPICS_BASE)/include
      INCLUDEPATH += $$(EPICS_BASE)/include/os/Darwin
      QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
      LIBS += -F$(QWTLIB) -framework qwt
      LIBS += ${EPICSLIB}/libca.dylib
      LIBS += ${EPICSLIB}/libCom.dylib
   }
}


ios {
   CONFIG += staticlib
}

