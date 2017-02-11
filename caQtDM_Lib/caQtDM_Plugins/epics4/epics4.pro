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

SOURCES += callbackThread.cpp epics4Requester.cpp
HEADERS += callbackThread.h epics4Requester.h

warning("epics4 was specified in qtdefs.pri, so build plugin with epics4 which will support all normative data types")


DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT
