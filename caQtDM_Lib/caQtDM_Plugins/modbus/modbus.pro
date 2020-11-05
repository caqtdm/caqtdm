include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui network
contains(QT_VER_MAJ, 5) {
    QT     += widgets concurrent
    QT += serialbus
}

CONFIG += warn_on
CONFIG += environment_Plugin

include (../../../caQtDM.pri)

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
INCLUDEPATH    += ../../../caQtDM_QtControls/src

HEADERS         = modbus_plugin.h modbus_decode.h ../controlsinterface.h \
    modbus_channeldata.h

SOURCES         = modbus_plugin.cpp modbus_decode.cpp \
    modbus_channeldata.cpp

TARGET          = modbus_plugin


