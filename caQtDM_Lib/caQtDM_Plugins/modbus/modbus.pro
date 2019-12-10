include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui network
contains(QT_VER_MAJ, 5) {
    QT     += widgets concurrent
}

CONFIG += warn_on
CONFIG += modbus_Plugin
include (../../../caQtDM.pri)

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = modbus_Plugin.h ../controlsinterface.h 
SOURCES         = modbus_Plugin.cpp 

TARGET          = modbus_Plugin


