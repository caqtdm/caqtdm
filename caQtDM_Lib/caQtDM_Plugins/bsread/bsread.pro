include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui network
contains(QT_VER_MAJ, 5) {
    QT     += widgets concurrent
}

CONFIG += warn_on
CONFIG += bsread_Plugin
include (../../../caQtDM.pri)

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
INCLUDEPATH    += ../../../caQtDM_QtControls/src
HEADERS         = bsread_Plugin.h ../controlsinterface.h \
    bsread_decode.h \
    bsread_channeldata.h \
    bsread_dispatchercontrol.h \
    bsread_wfhandling.h \
    bsread_wfconverter.h \
    bsread_wfblockconverter.h \
    bsread_wfconverterthread.h \
    bsread_internalchannel.h
SOURCES         = bsread_Plugin.cpp md5.cc \
    bsread_decode.cpp \
    bsread_channeldata.cpp \
    bsread_dispatchercontrol.cpp \
    bsread_wfhandling.cpp \
    bsread_wfconverterthread.cpp \
    bsread_internalchannel.cpp
TARGET          = bsread_Plugin


