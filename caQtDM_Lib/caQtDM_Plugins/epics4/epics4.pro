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


#if we want to use epics4, define it above
epics4: {
  warning("epics4 was specified in qtdefs.pri, so build plugin with epics4 (however old incompete version)")
  SOURCES +=  epics4Subs.cpp pvAccessImpl.cpp
  HEADERS +=  epics4Subs.h pvAccessImpl.h
  EPICS4LIB = /usr/local/epics/base-4.2.0/lib/SL6-x86/
  LIBS += -L$${EPICS4LIB} -Wl,-rpath,$${EPICS4LIB}  -lpvAccess -lpvData
  INCLUDEPATH += /usr/local/epics/base-4.2.0/include
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
}

