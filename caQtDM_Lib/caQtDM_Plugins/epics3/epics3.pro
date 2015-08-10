# this plugin does not need to be linked with the epics libraries
# while the main library caQtDM_Lib is already linked with them
TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
INCLUDEPATH    += $(EPICSINCLUDE)
INCLUDEPATH    += $(EPICSINCLUDE)/os/Linux
HEADERS         = epics3_plugin.h ../controlsinterface.h
SOURCES         = epics3_plugin.cpp epicsSubs.c
TARGET          = $$qtLibraryTarget(epics3_plugin)
DESTDIR         = $(CAQTDM_COLLECT)/controlsystems




