include (../caQtDM.pri)

TARGET = caQtDM
TEMPLATE = app

LIBS += -L$${QTCONTROLS} -Wl,-rpath,$${QTCONTROLS} -lcaQtDM_Lib

OBJECTS_DIR = ../obj1
DEFINES += SUPPORT=\\\"EPICS\\\"
