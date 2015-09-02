include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui
contains(QT_VER_MAJ, 5) {
    QT     += widgets
}
CONFIG += warn_on
CONFIG += epics3_plugin
include(../../../caQtDM.pri)

MOC_DIR = ./moc
VPATH += ./src

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
INCLUDEPATH    += $(EPICSINCLUDE)

HEADERS         = epics3_plugin.h ../controlsinterface.h
SOURCES         = epics3_plugin.cpp epicsSubs.c
TARGET          = epics3_plugin




