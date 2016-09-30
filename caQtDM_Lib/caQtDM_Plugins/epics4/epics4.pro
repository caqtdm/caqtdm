include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui
contains(QT_VER_MAJ, 5) {
    QT     += widgets
}
CONFIG += warn_on
CONFIG += release
CONFIG += epics4_plugin
include (../../../caQtDM.pri)

MOC_DIR = ./moc
VPATH += ./src

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = epics4_plugin.h ../controlsinterface.h
SOURCES         = epics4_plugin.cpp 
TARGET          = epics4_plugin


warning("epics4 was specified in qtdefs.pri, so build plugin with epics4 (however old incompete version)")
SOURCES +=  epics4Subs.cpp pvAccessImpl.cpp
HEADERS +=  epics4Subs.h pvAccessImpl.h
EPICS4LIB1 = /home/mezger/EPICS-CPP-4.6.0/pvAccessCPP/lib/SL6-x86
EPICS4LIB2 = /home/mezger/EPICS-CPP-4.6.0/pvDataCPP/lib/SL6-x86/
EPICS4LIB3 = /home/mezger/EPICS-CPP-4.6.0/pvaClientCPP/lib/SL6-x86/
LIBS += -L$${EPICS4LIB1} -Wl,-rpath,$${EPICS4LIB1}  -lpvAccess
LIBS += -L$${EPICS4LIB2} -Wl,-rpath,$${EPICS4LIB2}  -lpvData
LIBS += -L$${EPICS4LIB3} -Wl,-rpath,$${EPICS4LIB3}  -lpvaClient
INCLUDEPATH += /home/mezger/EPICS-CPP-4.6.0/pvDataCPP/include
INCLUDEPATH += /home/mezger/EPICS-CPP-4.6.0/pvAccessCPP/include
INCLUDEPATH += /home/mezger/EPICS-CPP-4.6.0/pvaClientCPP/include
INCLUDEPATH += /home/mezger/EPICS-CPP-4.6.0/normativeTypesCPP/src
INCLUDEPATH += $(EPICSINCLUDE)

!ios {
  unix:!macx {
  INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
  }
  macx {
      INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
  }
}

DEFINES += EPICS4

