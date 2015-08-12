# this plugin does not need to be linked with the epics libraries
# while the main library caQtDM_Lib is already linked with them
# not the case on mac ?!

QT += core gui
CONFIG += warn_on
CONFIG += release

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
INCLUDEPATH    += $(EPICSINCLUDE)

unix:!macx {
 INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
 LIBS += -L$(EPICSLIB) -Wl, -rpath,$(EPICSLIB) -lca -lCom
 LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
}

macx: {
 INCLUDEPATH   += $(EPICSINCLUDE)/os/Darwin
 LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
 LIBS += $$(EPICSLIB)/libca.dylib
 LIBS += $$(EPICSLIB)/libCom.dylib
 plugins.path = Contents/PlugIns/controlsystems
 plugins.files += $(CAQTDM_COLLECT)/controlsystems/libepics3_plugin.dylib
}

win32 {
  INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32
}

HEADERS         = epics3_plugin.h ../controlsinterface.h
SOURCES         = epics3_plugin.cpp epicsSubs.c
TARGET          = $$qtLibraryTarget(epics3_plugin)
DESTDIR         = $(CAQTDM_COLLECT)/controlsystems





