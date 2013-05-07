include (../caQtDM.pri)

TARGET = caQtDM_Hipa
TEMPLATE = app

#QTCONTROLS = /afs/psi.ch/user/m/mezger/workarea/ACS/mezger/caQtDM_Binaries

LIBS += -L$${QTCONTROLS} -Wl,-rpath,$${QTCONTROLS} -lcaQtDM_Lib_Hipa

OBJECTS_DIR = ../obj2
DEFINES += SUPPORT=\\\"EPICS_ACS\\\"

QMAKE_POST_LINK = cp ../caQtDM_Hipa ../../caQtDM_Binaries/
