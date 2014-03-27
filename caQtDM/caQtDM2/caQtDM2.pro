include (../caQtDM.pri)

TARGET = caQtDM_Hipa
TEMPLATE = app

LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib_Hipa

OBJECTS_DIR = ../obj2
DEFINES += SUPPORT=\\\"EPICS_ACS\\\"

QMAKE_POST_LINK = cp ../caQtDM_Hipa $(QTBASE)

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
