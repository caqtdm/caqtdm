# this plugin does not need to be linked with the epics libraries
# while the main library caQtDM_Lib is already linked with them
TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
INCLUDEPATH    += $(EPICSINCLUDE)
unix:!macx {
 INCLUDEPATH    += $(EPICSINCLUDE)/os/Linux
}
macx: {
 INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
}
win32 {
  INCLUDEPATH += $$(EPICS_BASE)/include/os/win32
}
HEADERS         = epics3_plugin.h ../controlsinterface.h
SOURCES         = epics3_plugin.cpp epicsSubs.c
TARGET          = $$qtLibraryTarget(epics3_plugin)
DESTDIR         = $(CAQTDM_COLLECT)/controlsystems




