include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui network
contains(QT_VER_MAJ, 5) {
    QT     += widgets concurrent
}

CONFIG += warn_on
CONFIG += environment_Plugin
include (../../../caQtDM.pri)

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src

HEADERS         = Environment_Plugin.h ../controlsinterface.h 

SOURCES         = Environment_Plugin.cpp

TARGET          = environment_plugin


