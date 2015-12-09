include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui
contains(QT_VER_MAJ, 5) {
    QT     += widgets
}

CONFIG += warn_on
CONFIG += bsread_Plugin
include (../../../caQtDM.pri)

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = bsread_Plugin.h ../controlsinterface.h \
    bsread_decode.h \
    bsread_channeldata.h
    #bsread_dispatchercontrol.h
SOURCES         = bsread_Plugin.cpp md5.cc \
    bsread_decode.cpp \
    bsread_channeldata.cpp
    #bsread_dispatchercontrol.cpp
TARGET          = bsread_Plugin


