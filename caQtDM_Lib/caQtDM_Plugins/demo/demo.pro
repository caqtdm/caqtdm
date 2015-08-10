
TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = demo_plugin.h ../controlsinterface.h
SOURCES         = demo_plugin.cpp
TARGET          = $$qtLibraryTarget(demo_plugin)
DESTDIR         = $(CAQTDM_COLLECT)/controlsystems

