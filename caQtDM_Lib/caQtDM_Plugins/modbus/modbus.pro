CAQTDM_TOP=../../..
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
QT += core gui network
contains(QT_VER_MAJ, 5) {
    QT     += widgets concurrent
    QT += serialbus
}

CONFIG += warn_on
CONFIG += modbus_Plugin
CONFIG += caQtDM_Installation
INSTALLTIONSUBDIR = controlsystems

include ($$CAQTDM_TOP/caQtDM.pri)

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


