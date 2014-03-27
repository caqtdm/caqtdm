include (../caQtDM_Lib.pri)

TARGET = caQtDM_Lib
OBJECTS_DIR = ../obj1
TEMPLATE = lib
unix:!macx {
  QMAKE_POST_LINK = cp ../libcaQtDM_Lib.so $(QTBASE)
}
macx: {
  QMAKE_POST_LINK = cp ../libcaQtDM_Lib.dylib $(QTBASE)
  INCLUDEPATH += $$(EPICS_BASE)/include
  INCLUDEPATH += $$(EPICS_BASE)/include/os/Darwin
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
  LIBS += -F$(QWTLIB) -framework qwt
  LIBS += ${EPICS_LIBS}/libca.dylib
  LIBS += ${EPICS_LIBS}/libCom.dylib
}

MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src
DESTDIR = ../
UI_DIR += ../
INCLUDEPATH += ../../caQtDM_QtControls/src



