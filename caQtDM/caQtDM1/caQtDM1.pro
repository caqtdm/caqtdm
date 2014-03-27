include (../caQtDM.pri)

TARGET = caQtDM
TEMPLATE = app

LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib

OBJECTS_DIR = ../obj1
DEFINES += SUPPORT=\\\"EPICS\\\"

unix:!macx {
  QMAKE_POST_LINK = cp ../caQtDM $(QTBASE)
}
macx: {
  QMAKE_POST_LINK = cp -R ../caQtDM.app ../../caQtDM_Binaries/
}


QMAKE_CXXFLAGS += "-g"
QMAKE_CFLAGS_RELEASE += "-g"

MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src
DESTDIR = ../
UI_DIR += ../src

INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += ../../caQtDM_QtControls/src
INCLUDEPATH += ../../caQtDM_Lib/src

RESOURCES += ../src/caQtDM.qrc
RC_FILE = ../src/caQtDM.rc
