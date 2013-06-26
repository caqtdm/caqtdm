include (../caQtDM.pri)

TARGET = caQtDM
TEMPLATE = app

LIBS += -L$${QTCONTROLS} -Wl,-rpath,$${QTCONTROLS} -lcaQtDM_Lib

OBJECTS_DIR = ../obj1
DEFINES += SUPPORT=\\\"EPICS\\\"

QMAKE_POST_LINK = cp ../caQtDM ../../caQtDM_Binaries/

QMAKE_CXXFLAGS += "-g"
QMAKE_CFLAGS_RELEASE += "-g"
