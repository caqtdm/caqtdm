include (../../../caQtDM_Viewer/qtdefs.pri)
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

warning("epics4 was specified in qtdefs.pri, so build plugin with epics4, not yet finished")

SOURCES +=  epics4Subs.cpp pvAccessImpl.cpp
HEADERS +=  epics4Subs.h pvAccessImpl.h


DEFINES += EPICS4

