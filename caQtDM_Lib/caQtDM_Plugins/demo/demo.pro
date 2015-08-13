include (../../../caQtDM_Viewer/qtdefs.pri)
CONFIG += warn_on
CONFIG += release
CONFIG += demo_plugin
include (../../../caQtDM.pri)

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = demo_plugin.h ../controlsinterface.h
SOURCES         = demo_plugin.cpp
TARGET          = $$qtLibraryTarget(demo_plugin)
DESTDIR         = $(CAQTDM_COLLECT)/controlsystems

