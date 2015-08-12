# this plugin has to be linked with epics4 libaries, while caQtDM_Lib is only
# linked with epics3
# the actual implementation of epics 4 is of an old type

QT += core gui
CONFIG += warn_on
CONFIG += release

include (../../../caQtDM_Viewer/qtdefs.pri)

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = epics4_plugin.h ../controlsinterface.h
SOURCES         = epics4_plugin.cpp 
TARGET          = $$qtLibraryTarget(epics4_plugin)
DESTDIR         = $(CAQTDM_COLLECT)/controlsystems

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


unix:!macx {
 INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
 LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
 LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
}

macx: {
        LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        LIBS += $$(EPICSLIB)/libca.dylib
        LIBS += $$(EPICSLIB)/libCom.dylib
        plugins.path = Contents/PlugIns/controlsystems
        plugins.files += $(CAQTDM_COLLECT)/controlsystems/libepics4_plugin.dylib
}
